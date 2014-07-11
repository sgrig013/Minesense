#include "StdAfx.h"
#include "ISensor.h"

using namespace std;

ISensor::ISensor(void)
{
}

ISensor::~ISensor(void)
{
}

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

string ISensor::GetLocalTime()
{
	char buf[80];
	time_t now = time(NULL); 
	struct tm localtm;
	localtime_s(&localtm, &now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &localtm);
	return buf;
}
