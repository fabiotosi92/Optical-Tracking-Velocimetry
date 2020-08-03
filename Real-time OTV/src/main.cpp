#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <unistd.h>
#include <math.h>           
#include <algorithm>
#include <chrono>
#include <array>
#include <cstdio>
#include <cstdlib>
#include "ops.h"
#include "utils.h"
#include "filter.h"
#include "reader.h"
#include "writer.h"

using namespace cv;
using namespace std;

void applyMask(Mat &img, Mat &mask);

int run(int argc, char** argv)
{
	initModule_nonfree();
	setUseOptimized(true);
    	

    //save argv to string
    string info_video = argv[1],
						calibration_file = argv[2],
						LK_file = argv[3],
						output_path = argv[4];
		
	if(!exists(output_path)){
		mkdir(output_path, true);}

	//summary
	stringstream summary, summary_temp;

	summary_temp << string(1,'\n') 
				 << " *** River Flow Velocity Estimation ***"
				 << string(2,'\n');

	//hardware info
	summary_temp << " * HARDWARE INFO" << string(2,'\n') 
				 << " - " << exec("lscpu | grep 'Architecture'")
				 << " - " << exec("lscpu | grep 'Model name'") 
				 << " - " << exec("lscpu | grep 'CPU max MHz'")
				 << " - " << exec("lscpu | grep 'CPU min MHz'")
				 << " - " << exec("lscpu | grep 'Thread(s) per core'")
				 << " - " << exec("cat /proc/meminfo | grep 'MemTotal'")
				 << string(2,'\n');

	//parameters from file
	float pixel_to_real, partial_min_angle, partial_max_angle, final_min_angle, final_max_angle, 		final_min_distance,max_features, region_step,resolution;
	int radius, maxLevel, maxCount, flags, height, width, total_frames,jump;
	double minEigThreshold, epsilon, fps;
	string feature_detector,directory_frame;
	
	//read Lucas-Kanede Parameter
	readLKParameters(LK_file, radius, maxLevel, maxCount, epsilon, flags, minEigThreshold);
	Size winSize(2*radius + 1, 2*radius + 1);
	TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS, maxCount, epsilon);
		
	//read Calibration Parameter
	readCalibration(calibration_file, pixel_to_real, partial_min_angle, partial_max_angle, 
					final_min_angle, final_max_angle, final_min_distance, max_features, 
					resolution, region_step, feature_detector);
		
	//read InfoVideo 
	readInfoVideo(info_video, fps, height, width, total_frames,directory_frame,jump);
	fps/=jump;
	total_frames=(int)total_frames/jump;
        
	//region step
	int step = (int)(region_step * resolution);

	summary_temp  << " * INFO VIDEO * " << endl
				  << " - Total frames: "            << total_frames       << endl
   	              << " - Frame Per Second (FPS): "  << fps                << endl
	              << " - Duration: "                << total_frames/fps   << " s" << endl
	              << string(2,'\n');

	summary << summary_temp.str();
	cout << summary_temp.str();
	summary_temp.str(string());
 
	//parameters
	Mat current_frame,previous_frame;

	//detector	
	Ptr<FeatureDetector> detector = FeatureDetector::create(feature_detector);
	if(feature_detector == "ORB" || feature_detector == "SIFT")
		detector->set("nFeatures", height * width);
	else if(feature_detector == "GFTT")
		detector->set("nfeatures", height * width);     

	
	//keypoints
	vector<KeyPoint> keypoints_current, keypoints_predicted, keypoints;  
	vector<KeyPoint> keypoints_start; 
	vector<int> time;
  
	//vector for Lucas-Kanede
	vector<uchar> status;
	vector<float> err;
	vector<Point2f> pt1, pt2;

	//velocity structure & structure for initialization 
	vector<vector<float>> velocity;
	vector<float> vel;

	//sub-regions
	vector<vector<float>> subregions_velocity;
	vector<float> subregions_trajectories;
	subRegionInit(subregions_velocity,subregions_trajectories, width, step);  
	
	//mask 
	Mat mask = (argc == 6) ? imread(argv[5], CV_LOAD_IMAGE_UNCHANGED) : Mat();
	string detected_mask = " - Mask: NOT DETECTED";
	if(mask.empty())
	{
		mask = Mat(height, width, CV_8U);
		mask.setTo(Scalar(1)); 
	}
	else
	{
		string mask_path = argv[5];
		mask/= 255;   
		detected_mask = " - Mask: " + mask_path;    	
	}

	string resolution_s;
	if(resolution == 1) 
		resolution_s = "Full Resolution";
	else if(resolution == 0.5) 
		resolution_s = "Half Resolution";
	else 
        	resolution_s = "Quarter Resolution";
		
	if(resolution != 1.0)
	{
		resize(mask,mask,Size(width, height));
	}


	summary_temp << " * FILTERING SETTINGS" << string(2,'\n')
				 << " - Partial filtering (min/max angle): "      << partial_min_angle  << "° to " << partial_max_angle << "°" << endl
				 << " - Final filtering (min/max angle): "        << final_min_angle    << "° to " << final_max_angle   << "°" << endl
				 << " - Final filtering (min covered distance): " << final_min_distance << " pixels" << endl
				 << string(2,'\n');

	summary_temp << " * STARTING COMPUTATION"                 << string(2,'\n')
				 << " - Feature Detector: "                   << feature_detector     << endl
				 << " - Tracker: Lucas-Kanade's Optical Flow" << endl 
				 << " - Searching Window Size: "              << "(" << 2 *radius + 1 << "," << 2 *radius + 1 << ")" << endl
				 << " - Maximal Pyramid Level Number: "       << maxLevel             << endl
				 << " - Max Features To Track: "              << max_features         << endl
				 << " - Resolution selected: "                << resolution_s  << " (" << width << "x" << height << ")" << endl
				 << detected_mask << endl
				 << string(2,'\n');

	summary << summary_temp.str();
	cout << summary_temp.str();
	summary_temp.str(string());

		
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();  

	cout << "\n - Execution Time -\n";	
	for(int idx=0;idx<total_frames;idx++)
        {
		cout << "[" << idx << "/" << total_frames << "]\r";
		cout.flush();            

		ostringstream os;
		os << directory_frame << "/img" << (idx*jump+1) << ".jpeg";
		string img = os.str();
		
		current_frame=imread(img);

		applyMask(current_frame, mask);
		
		detector->detect(current_frame,keypoints);
			
		int index = max_features - keypoints_current.size();
		index = (index < 0) ? (-index) : index;
		int size_kp = (keypoints.size()<index) ? 0 :(keypoints.size() - index);					
					
			
		for(size_t i=keypoints.size()-1; i>size_kp; i-- )
		{
			keypoints_current.push_back(keypoints[i]);
			keypoints_start.push_back(keypoints[i]);
			time.push_back(idx);

		}

		//initialization of vector<vector<float>> velocity		
		velocity.push_back(vel);
		
		if(idx!=0) 
		{
			KeyPoint::convert(keypoints_current, pt1);
			//Lucas-Kanade optical flow
			calcOpticalFlowPyrLK(previous_frame, current_frame, pt1, pt2, status, err, winSize, maxLevel, termcrit, OPTFLOW_LK_GET_MIN_EIGENVALS, minEigThreshold);

			keypoints_predicted.clear();
			for(size_t i = 0; i < pt2.size(); i++ ) 
			{
				keypoints_predicted.push_back(KeyPoint(pt2[i],1.f));
			}

			size_t i, k;
			for(i = k = 0; i < keypoints_current.size(); i++ )
			{ 
				if(!status[i] || !partialFiltering(keypoints_current[i], keypoints_predicted[i], partial_min_angle, partial_max_angle, maxLevel * (2*radius +1)/resolution))
				{	 
					if(finalFiltering(keypoints_start[i], keypoints_current[i], final_min_distance, final_min_angle, final_max_angle))
					{
						float velocity_i = getVelocity(keypoints_start[i], keypoints_current[i], pixel_to_real / resolution, (idx - time[i]), fps);
						//sub-regions computation
						int module_start = (int) keypoints_start[i].pt.x / step,
						    module_current = (int) keypoints_current[i].pt.x / step;

						if(module_start == module_current)
						{
							subregions_velocity[module_start].push_back(velocity_i);
							subregions_trajectories[module_start]++;
						}

						velocity[idx].push_back(velocity_i);
					}

				continue;
				}

				//update
				keypoints_current[k] = keypoints_current[i];
				keypoints_start[k] = keypoints_start[i];
				keypoints_predicted[k] = keypoints_predicted[i]; 
				time[k] = time[i];
				k++;
			}
		
			keypoints_current.resize(k);
			keypoints_start.resize(k);
			keypoints_predicted.resize(k);
			time.resize(k);
		}

		current_frame.copyTo(previous_frame);

		if(!keypoints_predicted.empty())
			swap(keypoints_current, keypoints_predicted);  
				
	}
        
	//results
	float avg, max, min, std_dev, count;
	computeStats(velocity, avg, max, min, std_dev, count);

	std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
	float execution_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/1000000.0;
	
	cout << endl;
	summary_temp <<  string(1,'\n') << " * RESULTS * " << string(2,'\n');
	
	if(count != 0)
	{
	summary_temp << " - Execution Time: " << execution_time << " s" << endl
			 << " - Computational Frame Rate: " << total_frames/execution_time << " f/s" << endl
			 << " - Max Velocity: " << max << " m/s," << endl
			 << " - Min Velocity: " << min << " m/s," << endl
			 << " - Avg Velocity: " << avg << " m/s," << endl
			 << " - Std Deviation: " << std_dev << " m/s," << endl
			 << " - Tot Filtered Trajectories: " << floatToString(count, 0) << endl
			 << string(2,'\n');
	}
	else
	{
	summary_temp << " - Execution Time: " << execution_time << " s" << endl
			 << " - Computational Frame Rate: " << total_frames/execution_time << " f/s" << endl
			 << " - Max Velocity: none," << endl
			 << " - Min Velocity: none," << endl
			 << " - Avg Velocity: none," << endl
			 << " - Std Deviation: none," << endl
			 << " - Tot Filtered Trajectories: " << floatToString(count, 0) << endl
			 << string(2,'\n');        	
	}

	summary << summary_temp.str();
	cout << summary_temp.str();
	summary_temp.str(string());

	//print results into CSV file
	writeStats(output_path,(int)count, avg,max,min,std_dev, execution_time);
	cout << " - Statistics saved in: " + output_path << string(2,'\n');

	//subregions
	vector<float> averages = getSubRegionAvg(subregions_velocity, subregions_trajectories);
	vector<float> std = getSubRegionStd(subregions_velocity, averages);
	writeSubRegionStats(averages, subregions_trajectories, std, width, step, output_path);

	cout << "\n * DONE!" << endl;

	return 0;
}

int main(int argc, char** argv)
{   
    if( argc != 5 && argc != 6)
    {
 		cout << "Usage: "
		<< argv[0]
		<< " [input info video path]"
		<< " [input calibration file]"
		<< " [input LK parameters file]"
		<< " [output path]"
		<< " [mask path (optional)]"
		<< endl;   	
    }
    else
    {
    	run(argc, argv);
    }


    return  0;
}

void applyMask(Mat &img, Mat &mask)
{
	vector<Mat> channels;
	split(img, channels);

	channels[0] = channels[0].mul(mask);
	channels[1] = channels[1].mul(mask);
	channels[2] = channels[2].mul(mask);

	merge(channels, img);
}
