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
