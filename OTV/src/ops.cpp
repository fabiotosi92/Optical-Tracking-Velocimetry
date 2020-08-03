#include "ops.h"

using namespace cv;
using namespace std;

/****************************
/* Single trajecetory stat
/****************************/

float getMagnitude(KeyPoint kp1, KeyPoint kp2)
{
	return sqrt(pow(kp2.pt.y - kp1.pt.y, 2) +
		        pow(kp2.pt.x - kp1.pt.x, 2));
}

float getAngle(KeyPoint kp1, KeyPoint kp2)
{
	return atan2(kp2.pt.y - kp1.pt.y, kp2.pt.x - kp1.pt.x) * 
	       180 / PI;
}

float getVelocity(KeyPoint kp1, KeyPoint kp2, float realDistancePixel, float time, float fps)
{
	return getMagnitude(kp1, kp2) * realDistancePixel * fps / time; 
}


/****************************
/* Sub-regions computation
/****************************/

void subRegionInit(vector<vector<float>>& input, int width, int step)
{
	int n_regions = ceil((float)width / step);

	for(int i=0; i < n_regions; i++)
	{
		vector<float> values; 
		input.push_back(values);
	}
}

void subRegionInit(vector<float>& input, int width, int step)
{
	int n_regions = ceil((float)width / step);

	for(int i=0; i < n_regions; i++)
	{
		input.push_back(0);
	}
}

vector<float> getSubRegionAvg(vector<vector<float>>& values, vector<float> trajectories)
{
	vector<float> averages;

	for(int i = 0; i < values.size(); i++)
	{
		averages.push_back(0);
		trajectories.push_back(0);
	}

	for(int i = 0; i < averages.size(); i++)
	{
		float sum = 0; 
		for(int j = 0; j < values[i].size(); j++)
			sum+=values[i][j];

		if(trajectories[i]!=0)
			averages[i] += sum/trajectories[i];
		else
			averages[i] = 0;
	}

	return averages;	
}

vector<float> getSubRegionStd(vector<vector<float>>& values, vector<float> averages)
{
	vector<float> std;

	for(int i = 0; i < values.size(); i++)
	{
		float std_dev = 0; 

		for(int j = 0; j < values[i].size(); j++)
		{
			std_dev+= pow(values[i][j] - averages[i], 2);
		}

		if(values[i].size() > 0)
			std.push_back(sqrt(std_dev/ values[i].size()));
		else 
			std.push_back(0);
	}

	return std;
}

/****************************
/* Statistics
/****************************/

void computeStats(vector<vector<float>> velocity, float &avg, float &max, float &min, float &std_dev, float &count)
{
	avg=0, max=0, min=FLT_MAX, std_dev=0, count = 0;

	//average, maximum, minimum 
	for(int i = 0; i < velocity.size(); i++)
	{
		for(int j = 0; j < velocity[i].size(); j++)
		{
			avg+= velocity[i][j];
			max = (velocity[i][j] > max) ? velocity[i][j] : max;
			min = (velocity[i][j] < min) ? velocity[i][j] : min;	
			count ++;		
		}
	}

	if(count >0)
	   avg/= count;

	//std deviation
	for(int i = 0; i < velocity.size(); i++)
	{
		for(int j = 0; j < velocity[i].size(); j++)
		{
			std_dev += pow(velocity[i][j] - avg, 2);
		}
	}

	if(count >0)
	   std_dev = sqrt(std_dev/count);
}

void onlineStats(vector<float>velocity, float &avg, float &max, float &min, float &std_dev, float &count)
{
	//average, maximum, minimum 
	for(int i = 0; i < velocity.size(); i++)
	{
		avg+= velocity[i];
		max = (velocity[i] > max) ? velocity[i] : max;
		min = (velocity[i] < min) ? velocity[i] : min;
		count++;			
	}

	//std deviation
	for(int i = 0; i < velocity.size(); i++)
	{
		std_dev += pow(velocity[i] - avg/count, 2);
	}
}

