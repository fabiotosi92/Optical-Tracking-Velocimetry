#include "utils.h"

using namespace cv;
using namespace std;

/****************************
/* String utilities
/****************************/

string zeroPadNum(int num, int width)
{
    ostringstream ss;
    ss << std::setw( width ) << std::setfill( '0' ) << num;
    return ss.str();
}

string join(string path1, string path2)
{
	return path1 + "/" + path2;
}

vector<string> split(const char *str, char c)
{
    vector<string> result;

    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(string(begin, str));
    } while (0 != *str++);

    return result;
}

string floatToString(float number, int precision)
{
    stringstream stream;
    stream << fixed << setprecision(precision) << number;
    return stream.str();    
}


/****************************
/* Linux - command utilities
/****************************/

bool exists(string filePath)
{
	struct stat myStat;
	return ((stat(filePath.c_str(), &myStat) == 0) && (((myStat.st_mode) & S_IFMT) == S_IFDIR));	
}

string exec(const char* cmd) 
{
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);

    if (!pipe) 
    	throw std::runtime_error("popen() failed!");

    while (!feof(pipe.get()))
    {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    }

    return result;
}

void execute(string command)
{
	system(command.c_str());
}

void mkdir(string folder, bool recursive)
{
	string command =  (recursive) ?  "mkdir -p " + folder 
	                              :  "mkdir " + folder;
	execute(command);
}


/****************************
/* Labels
/****************************/

vector<string> prepareLabels()
{
    vector<string> str;
    str.push_back("Line No. 1 ");
    str.push_back("Line No. 2 ");
    str.push_back("Line No. 3 ");
    str.push_back("Line No. 4 ");
    str.push_back("Line No. 5 ");
    return str;
}

void setLabel(cv::Mat& im, const string label, const Point &point)
{
    int fontface = FONT_HERSHEY_SIMPLEX;
    double scale = 0.5;
    int thickness = 1;
    int baseline = 0;

    Size text = getTextSize(label, fontface, scale, thickness, &baseline);
    rectangle(im, point + cv::Point(0, baseline), point + Point(text.width, -text.height), CV_RGB(0,0,0), CV_FILLED);
    putText(im, label, point, fontface, scale, CV_RGB(255,255,255), thickness, 8);
}

void putLabels(vector<string> labels, Mat &frame)
{
    for(int i = 0; i < labels.size();i++)
        setLabel(frame, labels[i].c_str(), Point2f(0, 20 + (i*20)));
}

/****************************
/* Video utils
/****************************/

Mat fillBackground(Mat &frame, Mat &trajectories, Mat &velocity)
{
	Mat img = Mat(frame.size(), CV_8UC3);
	img.setTo(Scalar(0,0,0));

	vector<Mat> channels(3),
	            channels_frame(3), 
	            channels_trajectories(3);

    split(img, channels);
    split(frame, channels_frame);
    split(trajectories, channels_trajectories);

	for(int row = 0; row < frame.rows; row++)
	{
		for(int col = 0; col < frame.cols; col++)
		{
			if(velocity.ptr<uchar>(row)[col] == 0)
			{
				channels[0].ptr<uchar>(row)[col] = channels_frame[0].ptr<uchar>(row)[col];
				channels[1].ptr<uchar>(row)[col] = channels_frame[1].ptr<uchar>(row)[col];
				channels[2].ptr<uchar>(row)[col] = channels_frame[2].ptr<uchar>(row)[col];
			}
			else
			{
				channels[0].ptr<uchar>(row)[col] = channels_trajectories[0].ptr<uchar>(row)[col];
				channels[1].ptr<uchar>(row)[col] = channels_trajectories[1].ptr<uchar>(row)[col];
				channels[2].ptr<uchar>(row)[col] = channels_trajectories[2].ptr<uchar>(row)[col];	
			}
		}
	}

	merge(channels, img);

	return img;
}


