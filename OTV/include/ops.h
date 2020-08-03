#include <opencv2/opencv.hpp>
#include <math.h> 

#define PI 3.14159265

/****************************
/* Single trajecetory stat
/****************************/

float getMagnitude(cv::KeyPoint kp1, cv::KeyPoint kp2);
float getAngle(cv::KeyPoint kp1, cv::KeyPoint kp2);
float getVelocity(cv::KeyPoint kp1, cv::KeyPoint kp2, float realDistancePixel, float time, float fps);

/****************************
/* Sub-regions computation
/****************************/

void subRegionInit(std::vector<std::vector<float>>& input, int width, int step);
void subRegionInit(std::vector<float>& input, int width, int step);
std::vector<float> getSubRegionAvg(std::vector<std::vector<float>>& values, std::vector<float> trajectories);
std::vector<float> getSubRegionStd(std::vector<std::vector<float>>& values, std::vector<float> averages);

/****************************
/* Statistics
/****************************/
void computeStats(std::vector<std::vector<float>> velocity, float &avg, float &max, float &min, float &std_dev, float &count);
void onlineStats(std::vector<float> velocity, float &avg, float &max, float &min, float &std_dev, float &count);
