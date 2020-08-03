#include "writer.h"

using namespace cv;
using namespace std;

/****************************
/* Video writer
/****************************/

void writeVideo(VideoCapture &capture, float resolution,
					  vector<vector<KeyPoint>> &keypoints_mem_current, 
	                  vector<vector<KeyPoint>> &keypoints_mem_predicted, 
	                  vector<vector<bool>> &valid,
	                  vector<vector<float>> velocity,
	                  vector<vector<float>> angle, 
	                  vector<vector<float>> distance,
	                  float max_final, float min_final,
	                  vector<vector<float>> &velocity_mem,
	                  string output_path)
{
    VideoWriter velocity_video;

    string velocity_path = join(output_path, "velocity.avi");

    //print stats on video
    vector<string> frame_labels = prepareLabels();

    double fps = capture.get(CV_CAP_PROP_FPS);
    float avg = 0, max= 0, min= FLT_MAX, std_dev= 0, count = 0;

    if (capture.set(CV_CAP_PROP_POS_FRAMES, 0) == false)
    {
         cout << "The video could not be rewinded" << std::endl;
         exit(1);
    }

	Mat velocity_img, normalized, 
	    colored, frame;

    //progress bar
    float progress = 0.0;

	for(int i = 0; i < keypoints_mem_current.size(); i++)
	{
        int barWidth = 62;
        cout << " [";

        int pos = barWidth * progress;
        for (int k = 0; k < barWidth; k++)   
        {
			if (k < pos) cout << "=";
			else if (k == pos) cout << ">";
			else cout << " ";
        }

        cout << "] (" << i+1 << "/" << keypoints_mem_current.size() << ")\r";
        cout.flush();

    	capture >> frame;

        if(resolution != 1.0)
        	resize(frame,frame,Size(frame.cols * resolution, frame.rows * resolution ));

    	if(i == 0)
    	{
			velocity_img = Mat(frame.size(), CV_32F),
			normalized, colored, frame;

			velocity_img.setTo(Scalar(0));
		    velocity_video.open(velocity_path, CV_FOURCC('M',  'J', 'P', 'G'),  fps , frame.size(), true);

			if (!velocity_video.isOpened())
			{
				cout << "Could not open the output video for write: " << endl;
			}

			if (!velocity_video.isOpened()){
				cout << "Could not open the output video for write: " << endl;
			}
    	}

		vector<Point2f> pt1, pt2;
		KeyPoint::convert( keypoints_mem_current[i], pt1);
		KeyPoint::convert( keypoints_mem_predicted[i], pt2);

		Point2f a(0.0f, 0.f), b(0.f, 1.0f), c(0.f, 2.0f);
		line(velocity_img, a, b, max_final, 1, CV_AA);
		line(velocity_img, b, c, min_final, 1, CV_AA);

		for(int j = 0; j < keypoints_mem_current[i].size(); j++)
		{
			if(valid[i][j] == true && !pt2.empty() && !pt1.empty())
			{
				line(velocity_img, pt1[j], pt2[j], velocity_mem[i][j], 1, CV_AA);				
			}
		}

		normalize(velocity_img, normalized, 0,  255, NORM_MINMAX);
	    normalized.convertTo(normalized, CV_8U);
	    applyColorMap(normalized, colored, COLORMAP_JET);

	    //frame stats 
		onlineStats(velocity[i], avg, max, min, std_dev, count);

	    frame_labels[0] = (count == 0) ? "velocity avg: none"           :  "velocity avg: "           + floatToString(avg/count, 4)           + " m/s";
	    frame_labels[1] = (count == 0) ? "velocity max: none"           :  "velocity max: "           + floatToString(max, 4)                 + " m/s"; 
	    frame_labels[2] = (count == 0) ? "velocity min: none"           :  "velocity min: "           + floatToString(min, 4)                 + " m/s";
	    frame_labels[3] = (count == 0) ? "standard deviation: none"     :  "standard deviation: "     + floatToString(sqrt(std_dev/count), 4) + " m/s";
	    frame_labels[4] = (count == 0) ? "number of trajectories: none" :  "number of trajectories: " + floatToString(count, 1);  
	    
	    Mat filled = fillBackground(frame, colored, normalized);
	    putLabels(frame_labels, filled);

	    velocity_video << filled;

	    progress += (1.0/(float)keypoints_mem_current.size());
	}
}


/****************************
/* File writer
/****************************/

void writeStats(string output_path, string summary, vector<vector<float>> velocity, vector<vector<float>> angle, vector<vector<float>> distance,
	          float avg, float max, float min, float std_dev, float computation_time)
{
    string file_name_verbose = join(output_path, "stats_verbose.csv");
    string file_name_synthesis = join(output_path, "stats_synthesis.csv");

	ofstream file_verbose, file_synthesis;
	file_verbose.open (file_name_verbose);
	file_synthesis.open (file_name_synthesis);

	file_verbose << summary;
	file_verbose << "* TRAJECTORIES" << endl;
	file_verbose << "  N. trajectory,  Velocity (m/s),  Angle (deg),  Distance (m),  Time (s)" << endl;

	int count = 0;

	for(int i = 0; i < velocity.size(); i++)
	{
		for(int j = 0; j < velocity[i].size(); j++)
		{
			count++;

			file_verbose << count << ","
			     << velocity[i][j] << ","
			     << angle[i][j] << " ,"
			     << distance[i][j] << ","
			     << distance[i][j]/velocity[i][j] << ","
			     << endl;
		}
	}

	file_synthesis << "avg," << avg << endl
                   << "std_dev," << std_dev << endl
                   << "tot_trajectories," << count << endl
                   << "time," << computation_time;

	file_verbose.close();
	file_synthesis.close();
}

void writeSubRegionStats(vector<float>& averages, vector<float>& trajectories, vector<float>& std, int width, int step, string output_path)
{
	string csv = output_path + "/sub_regions.csv";
	ofstream file;
	file.open (csv);

	int n_regions = width / step;

	file << "Interval, ";

	int previous = 0, current = step;

	for(int i = 0; i < n_regions; i++)
	{

		file << "[" << to_string(previous) << "-" << to_string(current) << "], ";
		previous = current;
		current += step;
	}

	if(width - step*n_regions > 0)
		file << "[" << to_string(step*n_regions) << "-" << to_string(width) << "], ";

	file << endl;

	file << "Velocity avg (m/s), ";
	for(int i = 0; i < averages.size();i++)
		file << to_string(averages[i]) << ", ";

	file << endl << "Standard Deviation (m/s), ";
	for(int i = 0; i < std.size();i++)
		file << to_string(std[i]) << ", ";

	file << endl << "N. trajectories, ";
	for(int i = 0; i < trajectories.size();i++)
		file << to_string(trajectories[i]) << ", ";

	file.close();
}
