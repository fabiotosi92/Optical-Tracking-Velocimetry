#include "reader.h"

using namespace cv;
using namespace std;

/****************************
/* File reader
/****************************/

void readCalibration(string file_name, float &pixel_to_real, 
	float &partial_min_angle, float &partial_max_angle,
	float &final_min_angle, float &final_max_angle, float &final_min_distance, 
    float &max_features, float &resolution, float &region_step, string &feature_detector)
{
    ifstream file(file_name);
    string str; 
    while (getline(file, str))
    {
      vector<string> splitted = split(str.c_str(), ' ');
      if(splitted[0] == "pixel_to_real") 
      	pixel_to_real = stof(splitted[1]);
      else if(splitted[0] == "partial_min_angle") 
      	partial_min_angle = stof(splitted[1]);
      else if(splitted[0] == "partial_max_angle") 
      	partial_max_angle = stof(splitted[1]);
      else if(splitted[0] == "final_min_angle") 
      	final_min_angle = stof(splitted[1]);
      else if(splitted[0] == "final_max_angle") 
      	final_max_angle = stof(splitted[1]);
      else if(splitted[0] == "final_min_distance") 
      	final_min_distance = stof(splitted[1]);
      else if(splitted[0] == "region_step") 
      	region_step = stof(splitted[1]);
      else if(splitted[0] == "max_features")
        max_features = stof(splitted[1]);
      else if(splitted[0] == "resolution") 
      {
      	if(splitted[1] == "F") 
      		resolution = 1.0;
      	else if(splitted[1] == "H") 
      		resolution = 0.5;
      	else if(splitted[1] == "Q") 
      		resolution = 0.25;
      	else exit(-1);
      } 
      else if(splitted[0] == "detector") 
      	feature_detector = splitted[1];
    }

    final_min_distance*=resolution;

    file.close();
}

//lukas-kanade parameters
void readLKParameters(string file_name, int& radius, 
	                  int& maxLevel, int& maxCount, double& epsilon, 
	                  int& flags, double& minEigThreshold)
{
    ifstream file(file_name);
    string str; 
    while (getline(file, str))
    {
      vector<string> splitted = split(str.c_str(), ' ');

      if(splitted[0] == "radius")               
      	radius = stoi(splitted[1]);
      else if(splitted[0] == "maxLevel")        
      	maxLevel = stoi(splitted[1]);
      else if(splitted[0] == "maxCount")        
      	maxCount = stoi(splitted[1]);
      else if(splitted[0] == "epsilon")         
      	epsilon = stod(splitted[1]);
      else if(splitted[0] == "flags")           
      	flags = stof(splitted[1]);
      else if(splitted[0] == "minEigThreshold") 
      	minEigThreshold = stod(splitted[1]);
    }

    file.close();	
}
