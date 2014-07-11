#pragma once
#include "stdafx.h"

class ISensor
{
public:
	ISensor(void);
	virtual ~ISensor(void);

	virtual bool Start() = 0;
	virtual std::string GenerateData() = 0;
	virtual bool SendData(std::string) = 0;

	static std::string GetRandomNumericData(int length);
	static std::string GetLocalTime();

	enum SensorType
	{
		XRF,
		EMS,
		LIBS
	};

	enum DataType
	{
		Temperature, // Celsius/Fahrenheit
		Voltage,     // Volt
		Humidity,	 // RH (relative humidity)
		Frequency,	 // Hz
		Wavelength,	 // nm 
		Energy,		 // eV (electronvolts in Atom physics) 
		Pressure	 // Pa 
	};

	enum DataUnit
	{
		Celsius,
		Fahrenheit,
		Volt,
		EVolt,
		RH,
		Hz,
		NM,
		Pa
	};

	typedef struct _SensorMetaData
	{
		std::string	version;
		SensorType	sensor;
		std::string	sensorLocation;
		DataType	metric;
		DataUnit	unit;
	} SensorMetaData;

	typedef struct _SensorReading
	{
		DataType	sup_data;
		DataUnit	sup_unit;
		std::string	timestamp;
		std::string	data;
	} SensorReading;

};

