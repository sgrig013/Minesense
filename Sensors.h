#pragma once
#include "ISensor.h"

// These are Sensor specific classes 
// derived from base ISensor class 
class SensorXRF : 
	public ISensor
{
public:
	SensorXRF(void);
	~SensorXRF(void);

	bool Start();
	std::string GenerateData();
	bool SendData(std::string);

//private:
//	std::string m_strData;
};

class SensorEMS :
	public ISensor
{
public:
	SensorEMS(void);
	~SensorEMS(void);

	bool Start();
	std::string GenerateData();
	bool SendData(std::string);
	
};

class SensorLIBS :
	public ISensor
{
public:
	SensorLIBS(void);
	~SensorLIBS(void);

	bool Start();
	std::string GenerateData();
	bool SendData(std::string);
};
