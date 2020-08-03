#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
//#include <opencv2/nonfree/features2d.hpp>
//#include <opencv2/nonfree/nonfree.hpp>
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
    	//cv::initModule_nonfree();

        //save argv to string
        string input_video = argv[1],
        	   calibration_file = argv[2],
        	   LK_file = argv[3],
               output_path = argv[4];

	    if(!exists(output_path)){
	   		mkdir(output_path, true);}

        //summary
        stringstream summary, summary_temp;

        summary_temp << string(2,'\n') 
                     << " *** River Flow Velocity Estimation ***"
                     << string(3,'\n');

        //hardware info
        summary_temp << " * HARDWARE INFO" << string(2,'\n') 
                     << " - " << exec("lscpu | grep 'Architecture'")
                     << " - " << exec("lscpu | grep 'Model name'") 
                     << " - " << exec("lscpu | grep 'CPU max MHz'")
                     << " - " << exec("lscpu | grep 'CPU min MHz'")
                     << " - " << exec("lscpu | grep 'Thread(s) per core'")
                     << " - " << exec("cat /proc/meminfo | grep 'MemTotal'")
                     << string(2,'\n');

        //open the video file
        VideoCapture capture(input_video);

        summary_temp << " * INFO VIDEO" << string(2,'\n');

        //check initialization
        if(!capture.isOpened()) 
        {
			cout <<" *** Error: could not initialize capturing ***"
			     << endl;
            return  -1;
        }
        else
        {
			summary_temp << " - File name: "
			             << input_video
			             << endl;
        }

        //frames extraction
        int total_frames = int(capture.get(CV_CAP_PROP_FRAME_COUNT));

        //frames per second
        double fps = capture.get(CV_CAP_PROP_FPS);

        //parameters from file
        float pixel_to_real, partial_min_angle, partial_max_angle, 
              final_min_angle, final_max_angle, final_min_distance, 
              max_features, region_step, resolution;

        int radius, maxLevel, maxCount, flags;
        double minEigThreshold, epsilon;

		readLKParameters(LK_file, radius, maxLevel, maxCount, epsilon, flags, minEigThreshold);

        Size winSize(2*radius + 1, 2*radius + 1);
		TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS, maxCount, epsilon);

        string feature_detector;

        readCalibration(calibration_file, pixel_to_real, partial_min_angle, partial_max_angle, 
        	                final_min_angle, final_max_angle, final_min_distance, max_features, 
        	                resolution, region_step, feature_detector);

        //image resolution
        int height = lround(capture.get(CV_CAP_PROP_FRAME_HEIGHT) * resolution), 
            width = lround(capture.get(CV_CAP_PROP_FRAME_WIDTH) * resolution);

        //region step
        int step = (int)(region_step * resolution);

		summary_temp  << " - Total frames: "            << total_frames       << endl
			          << " - Frame Per Second (FPS): "  << fps                << endl
		              << " - Duration: "                << total_frames/fps   << " s" << endl
		              << " - Resolution: "              << width              << "x" << height << endl 
		              << string(2,'\n');

		summary << summary_temp.str();
		cout << summary_temp.str();
		summary_temp.str(string());

        //parameters
        Mat current_frame, previous_frame;

        //index current frame
        int frame_idx = 0;

        //keypoints
        vector<KeyPoint> keypoints_current, keypoints_predicted, keypoints;  
        vector<KeyPoint> keypoints_start, keypoints_final; 
        vector<int> time;

        //keypoint's storage
        vector<vector<KeyPoint>> keypoints_mem_current, keypoints_mem_predicted;
        vector<vector<bool>> valid; 
        vector<vector<int>> path;

        //magnitude/angle's storage
        vector<vector<float>> velocity_mem;

        //velocity result
        vector<vector<float>> velocity, angle, distance;

        //sub-regions
        vector<vector<float>> subregions_velocity;
        vector<float> subregions_trajectories;
        subRegionInit(subregions_velocity, width, step);
        subRegionInit(subregions_trajectories, width, step);

        //initialization
        for( size_t i = 0; i < total_frames; i++)
        {
        	vector<bool> valid_at_idx;
        	vector<float> velocity_at_idx;
        	vector<float> vel, ang, dist;
        	vector<int> path_at_idx;

			valid.push_back(valid_at_idx);
			velocity_mem.push_back(velocity_at_idx);
			velocity.push_back(vel);
			angle.push_back(ang);
			distance.push_back(dist);
			path.push_back(path_at_idx);
        }

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
       
        if (capture.set(CV_CAP_PROP_POS_FRAMES, 0) == false)
        {
			cout << "The video could not be rewinded" << endl;
			exit(1);
        }

        string resolution_s;

        if(resolution == 1) resolution_s = "Full Resolution";
        else if(resolution == 0.5) resolution_s = "Half Resolution";
        else resolution_s = "Quarter Resolution";

        summary_temp << " * FILTERING SETTINGS" << string(2,'\n')
        	         << " - Partial filtering (min/max angle): "          << partial_min_angle    << "° to "   << partial_max_angle << "°" << endl
        	         << " - Final filtering (min/max angle): "            << final_min_angle      << "° to "   << final_max_angle   << "°" << endl
        	         << " - Final filtering (min covered distance): "     << final_min_distance   << " pixels" << endl
        	         << string(2,'\n');

        summary_temp << " * STARTING COMPUTATION"                 << string(2,'\n')
        	         << " - Feature Detector: "                   << feature_detector     << endl
        	         << " - Tracker: Lucas-Kanade's Optical Flow" << endl 
        	         << " - Searching Window Size: "              << "(" << 2 *radius + 1 << "," << 2 *radius + 1 << ")" << endl
        	         << " - Maximal Pyramid Level Number: "       << maxLevel             << endl
        	         << " - Max Features To Track: "              << max_features         << endl
        	         << " - Resolution selected: "                << resolution_s 
        	         << " (" << width << "x" << height << ")" << endl
        	         << detected_mask << endl
        	         << string(2,'\n');

		summary << summary_temp.str();
		cout << summary_temp.str();
		summary_temp.str(string());

        //progress bar
        float progress = 0.0;

        //computing time
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        //detector
        Ptr<FeatureDetector> detector = FeatureDetector::create(feature_detector);

        if(feature_detector == "ORB" || feature_detector == "SIFT")
        {
        	detector->set("nFeatures", current_frame.rows * current_frame.cols);
        }
        else if(feature_detector == "GFTT")
        {
        	detector->set("nfeatures", current_frame.rows * current_frame.cols);
        }

        while(progress < 1.0)
        {
            int barWidth = 62;
            cout << " [";

            int pos = barWidth * progress;
            for (int i = 0; i < barWidth; i++)   
            {
				if (i < pos) cout << "=";
				else if (i == pos) cout << ">";
				else cout << " ";
            }

            cout << "] (" << frame_idx << "/" << total_frames << ")\r";
            cout.flush();
        
            capture >> current_frame;

            if(current_frame.empty())
                break;

            if(resolution != 1.0)
            {
	            resize(current_frame,current_frame,Size(current_frame.cols * resolution, current_frame.rows * resolution));
	            resize(mask,mask,Size(current_frame.cols, current_frame.rows));
            }

            applyMask(current_frame, mask);

            //progress bar update
            progress += (1.0/(float)total_frames);

			//detection
  			detector->detect(current_frame, keypoints);

			if(previous_frame.empty())
			{
				for(size_t i = 0; i < keypoints.size(); i++ )
				{
					if(keypoints_current.size() < max_features)
					{
						keypoints_current.push_back(keypoints[i]);
						keypoints_start.push_back(keypoints[i]);
						time.push_back(frame_idx);
						valid[frame_idx].push_back(false);
						velocity_mem[frame_idx].push_back(0);
						path[frame_idx].push_back(i);
					}
				}
			}
			else
			{   
				for(size_t i = keypoints.size() - 1; i >0; i-- )
				{
					if(keypoints_current.size() < max_features)
					{
						keypoints_current.push_back(keypoints[i]);
					    keypoints_start.push_back(keypoints[i]);
						time.push_back(frame_idx);
						valid[frame_idx].push_back(false);
						velocity_mem[frame_idx].push_back(0);		
					}
				}	
			}

			if(!previous_frame.empty()) 
			{
				vector<uchar> status;
                vector<float> err;

                vector<Point2f> pt1, pt2;
				KeyPoint::convert(keypoints_current, pt1);

				//Lucas-Kanade optical flow
                calcOpticalFlowPyrLK(previous_frame, current_frame, pt1, pt2, status, err, winSize, maxLevel, termcrit, OPTFLOW_LK_GET_MIN_EIGENVALS, minEigThreshold);

                keypoints_predicted.clear();
				for( size_t i = 0; i < pt2.size(); i++ ) 
				{
				  keypoints_predicted.push_back(KeyPoint(pt2[i], 1.f));
				}

				size_t i, k;

                for(i = k = 0; i < keypoints_current.size(); i++ )
                { 
					if(!status[i] || 
						!partialFiltering(keypoints_current[i], keypoints_predicted[i], partial_min_angle, partial_max_angle, maxLevel * (2*radius +1)/resolution))
					{	      
						if(finalFiltering(keypoints_start[i], keypoints_current[i], final_min_distance, final_min_angle, final_max_angle))
						{
							float velocity_i = getVelocity(keypoints_start[i], keypoints_current[i], pixel_to_real / resolution, frame_idx - time[i], fps);
							float angle_i = getAngle(keypoints_start[i], keypoints_current[i]);
							
							//sub-regions computation
							int module_start = (int) keypoints_start[i].pt.x / step,
							    module_current = (int) keypoints_current[i].pt.x / step;

							if(module_start == module_current)
							{
								subregions_velocity[module_start].push_back(velocity_i);
								subregions_trajectories[module_start]++;
							}

							//update storage
							int pos = i;
							for(int j = frame_idx - 1; j >= time[i]; j--)
							{
								valid[j][pos] = true;
								velocity_mem[j][pos] = velocity_i;
								pos = path[j][pos];
							}

							velocity[frame_idx].push_back(velocity_i);
							angle[frame_idx].push_back(angle_i);
							distance[frame_idx].push_back(velocity_i * (frame_idx - time[i])/fps);
						}

						continue;
					}

					//update
                    keypoints_current[k] = keypoints_current[i];
                    keypoints_start[k] = keypoints_start[i];
                    keypoints_predicted[k] = keypoints_predicted[i]; 
                    path[frame_idx].push_back(i);
                    velocity_mem[frame_idx].push_back(0);
                    valid[frame_idx].push_back(false);
                    time[k] = time[i];
                    k++;
				}

				stringstream ss;
				ss << frame_idx;
				string str = ss.str();

				keypoints_current.resize(k);
				keypoints_start.resize(k);
				keypoints_predicted.resize(k);
				time.resize(k);
			}

			current_frame.copyTo(previous_frame);

			//update keypoint's storage
			keypoints_mem_current.push_back(keypoints_current);
			keypoints_mem_predicted.push_back(keypoints_predicted);

			if(!keypoints_predicted.empty())
				swap(keypoints_current, keypoints_predicted);  

            frame_idx++;                                              
        }

        //results
    	float avg, max, min, std_dev, count;
		computeStats(velocity, avg, max, min, std_dev, count);

		std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
		float execution_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/1000000.0;

		cout << endl;
        summary_temp <<  string(1,'\n') 
                     << " * RESULTS" 
                     << string(2,'\n');

        if(count != 0)
        {
			summary_temp << " - Execution Time: " << execution_time << " s" << endl
			             << " - Computational Frame Rate: " << total_frames/execution_time << " f/s" << endl
			             << " - Max Velocity: " << max << " m/s," << endl
			             << " - Min Velocity: " << min << " m/s," << endl
			             << " - Avg Velocity: " << avg << " m/s," << endl
			             << " - Std Deviation: " << std_dev << " m/s," << endl
			             << " - Tot Filtered Trajectories: " << floatToString(count, 1) 
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
			             << " - Tot Filtered Trajectories: " << floatToString(count, 1) 
			             << string(2,'\n');        	
        }

		summary << summary_temp.str();
		cout << summary_temp.str();
		summary_temp.str(string());

	    cout << " * DRAWING RESULTS" << string(2,'\n');

	    //visualization
		writeVideo(capture, resolution, keypoints_mem_current, keypoints_mem_predicted, valid, velocity, angle, distance, max, min, velocity_mem, output_path);

        //print results into CSV file
	    writeStats(output_path, summary.str(), velocity, angle, distance, avg, max, min, std_dev, execution_time);
	    cout << " - Statistics saved in: " + output_path << string(2,'\n');

		//subregions
		vector<float> averages = getSubRegionAvg(subregions_velocity, subregions_trajectories);
		vector<float> std = getSubRegionStd(subregions_velocity, averages);
		writeSubRegionStats(averages, subregions_trajectories, std, width, step, output_path);

		cout << " * DONE!" << endl;

		return 0;
}

int main(int argc, char** argv)
{   
    if( argc != 5 && argc != 6)
    {
 		cout << "Usage: "
		<< argv[0]
		<< " [input video path]"
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
