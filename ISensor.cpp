#include "StdAfx.h"
#include "ISensor.h"

using namespace std;

// This is implementation of sensor base class. 
// Implementation includes generic helper methods 
// to use in derivred classes. 
ISensor::ISensor(void)
{
}

ISensor::~ISensor(void)
{
}

// Generates string with random numbers
string ISensor::GetRandomNumericData(int length)
{
	double xRan;
	stringstream ss;

	srand((unsigned)time(0)); 
	for (int i=0; i<length; i++)
	{
		xRan=rand()%10;
		ss << xRan;
	}
	return ss.str();
}

// Obtains and returns string with current local time 
// in user friendly format
string ISensor::GetLocalTime()
{
	char buf[80];
	time_t now = time(NULL); 
	struct tm localtm;
	localtime_s(&localtm, &now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &localtm);
	return buf;
}
