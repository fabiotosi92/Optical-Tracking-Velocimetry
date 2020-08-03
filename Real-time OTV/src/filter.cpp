#include "filter.h"
#include "ops.h"

using namespace cv;
using namespace std;


bool partialFiltering(KeyPoint kp1, KeyPoint kp2, float minAngle, float maxAngle, float maxDistance)
{
	float magnitude = sqrt(((kp2.pt.y - kp1.pt.y)*(kp2.pt.y - kp1.pt.y)) + ((kp2.pt.x - kp1.pt.x)*(kp2.pt.x - kp1.pt.x))); //only to limit the research window
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
	float magnitude = sqrt(((kp2.pt.y - kp1.pt.y)*(kp2.pt.y - kp1.pt.y)) + ((kp2.pt.x - kp1.pt.x)*(kp2.pt.x - kp1.pt.x)));
	if(magnitude < minMagnitude)
		return false;
	
	//angle filtering
	float angle = getAngle(kp1, kp2);
	if(angle > minAngle && angle < maxAngle)
	  return true; 

	return false;
}
