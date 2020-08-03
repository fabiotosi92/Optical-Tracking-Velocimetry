 #include "ops.h"

using namespace cv;
using namespace std;

/****************************
/* Single trajecetory stat
/****************************/
float getAngle(KeyPoint kp1, KeyPoint kp2)
{
	return atan2(kp2.pt.y - kp1.pt.y, kp2.pt.x - kp1.pt.x) *180 / PI;
}

float getVelocity(KeyPoint kp1, KeyPoint kp2, float realDistancePixel, float time, float fps)
{	
	float magnitude= sqrt(((kp2.pt.y - kp1.pt.y)*(kp2.pt.y - kp1.pt.y)) + ((kp2.pt.x - kp1.pt.x)*(kp2.pt.x - kp1.pt.x)));
	return magnitude * realDistancePixel * fps / time; 
}


/****************************
/* Sub-regions computation
/****************************/

void subRegionInit(vector<vector<float>>& input, vector<float>& input2, int width, int step)
{
	int n_regions = ceil((float)width / step);
	vector<float> values; 
	
	for(int i=0; i < n_regions; i++)
	{
		input.push_back(values);
		input2.push_back(0);
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
			sum += values[i][j];
	

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
	
	for(size_t i = 0; i < values.size(); i++)
	{
		float std_dev = 0; 
		
		for(int j = 0; j < values[i].size(); j++)
		{
			std_dev+= (values[i][j] - averages[i])*(values[i][j] - averages[i]);
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
	for(vector<float> vel_vec : velocity)
	{
		for(float vel : vel_vec)
		{
			if(vel>0)
			{
			avg += vel;
			max = (vel > max) ? vel : max;
			min = (vel < min) ? vel : min;	
			count ++;		
			}
		}
	}

	if(count >0)
	   avg/= count;

	//std deviation
	for(vector<float> vel_vec : velocity)
	{
		for(float vel : vel_vec)
		{
			std_dev += (vel - avg)*(vel - avg);
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
		std_dev += (velocity[i] - avg/count)*(velocity[i] - avg/count);
	}
}

