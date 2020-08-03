#include "filter.h"
#include "ops.h"

using namespace cv;
using namespace std;


bool partialFiltering(KeyPoint kp1, KeyPoint kp2, float minAngle, float maxAngle, float maxDistance)
{
	float magnitude = getMagnitude(kp1, kp2);//only to limit the research window
	if(magnitude > maxDistance)
		return false;

	float angle = getAngle(kp1, kp2); 
	if(angle > minAngle && angle < maxAngle)
	  return true;

	return false;
}

bool finalFiltering(KeyPoint kp1, KeyPoint kp2, float minMagnitude, float minAngle, float maxAngle)
{
	//magnitude filtering
	float magnitude = getMagnitude(kp1, kp2);
	if(magnitude < minMagnitude)
		return false;
	
	//angle filtering
	float angle = getAngle(kp1, kp2);
	if(angle > minAngle && angle < maxAngle)
	  return true; 

	return false;
}
