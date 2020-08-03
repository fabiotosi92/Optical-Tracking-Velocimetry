#include "writer.h"

using namespace cv;
using namespace std;

/****************************
/* File writer
/****************************/

void writeStats(string output_path, int count, float avg, float max, float min, float std_dev, float computation_time)
{
    string file_name_synthesis = join(output_path, "stats_synthesis.csv");

	ofstream file_synthesis;
	file_synthesis.open (file_name_synthesis);

	file_synthesis << "avg," << avg << endl
				   << "min velocity," << min << endl
				   << "max velocity," << max << endl
                   << "std_dev," << std_dev << endl
                   << "tot_trajectories," << count << endl
                   << "time," << computation_time;

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
