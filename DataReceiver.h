#pragma once
#include "ISensor.h"

// Only one instance of the receiver class could be created
// which is supported by singleton creational pattern
class DataReceiver
{
private:
    static DataReceiver *pReceiver;
	DataReceiver(void);
public:
	~DataReceiver(void);

	static DataReceiver* GetInstance();
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
	// string to hold data received from a sensor
	std::string m_sRcvData;

};

