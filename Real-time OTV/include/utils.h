#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include <stdexcept>
#include <memory>

/****************************
/* String utilities
/****************************/
std::string zeroPadNum(int num, int width);
std::string join(std::string path1, std::string path2);
std::vector<std::string> split(const char *str, char c);
std::string floatToString(float number, int precision);

/****************************
/* Linux - command utilities
/****************************/
bool exists(std::string filePath);
std::string exec(const char* cmd);
void execute(std::string command);
void mkdir(std::string folder, bool recursive);
