// MinesenseTestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DataReceiver.h"
#include "Sensors.h"
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	// Create Receiver and start to wait for a sensor data
	DataReceiver* pRcv = DataReceiver::GetInstance();
	if (!pRcv->Start())
	{
		cout << "Failed to start data receiver!" << endl;
		system("pause");
		return 1;
	}
	// Create Sensor 1 and start sending data
	SensorXRF xrf;
	if (!xrf.Start())
	{
		system("pause");
		return 1;
	}
	if (pRcv->NumerOfSensorsProcessed() != 1)
	{
		system("pause");
		return 1;
	}
	// Create Sensor 2 and start
	SensorEMS ems;
	if (!ems.Start())
	{
		system("pause");
		return 1;
	}
	if (pRcv->NumerOfSensorsProcessed() != 2)
	{
		system("pause");
		return 1;
	}
	 
	// Create Sensor 3 and start
	SensorLIBS libs;
	if (!libs.Start())
	{
		system("pause");
		return 1;
	}
	if (pRcv->NumerOfSensorsProcessed() != 2)
	{
		system("pause");
		return 1;
	}

	//  Stop Receiver if data received from all three sensors

	system("pause");
	return 0;
}

