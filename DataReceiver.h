#pragma once
#include "ISensor.h"

class DataReceiver
{
public:
	DataReceiver(void);
	~DataReceiver(void);

	bool Start();
	bool ReceiveData();
	bool SaveData(int sensorIndex);
	int  NumerOfSensorsProcessed();
	void CloseEvents();

	friend DWORD ThreadProc(LPVOID);

	inline std::string SensorTypeToString(ISensor::SensorType type);
	inline std::string DataTypeToString(ISensor::DataType type);
	inline std::string DataUnitToString(ISensor::DataUnit unit);

private:
	// number of sensor's data processed
	int m_nProcessed;
	std::string m_sRcvData;

};

