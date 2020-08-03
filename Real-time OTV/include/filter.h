#include <opencv2/opencv.hpp>

bool partialFiltering(cv::KeyPoint kp1, cv::KeyPoint kp2, float minAngle, float maxAngle, float maxDistance);
bool finalFiltering(cv::KeyPoint kp1, cv::KeyPoint kp2, float minMagnitude, float minAngle, float maxAngle);
