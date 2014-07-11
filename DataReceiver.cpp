#include "StdAfx.h"
#include "DataReceiver.h"
#include <fstream>
#include <map>
using namespace std;
using namespace constants;

HANDLE ghSentEvent[NUM_SENSORS];
HANDLE ghReceivedEvent[NUM_SENSORS];
HANDLE ghThread;
DataReceiver* DataReceiver::pReceiver= NULL;

// This is implementation of the server class to wait for notifications from sensors, 
// receive sensor reading data and save it to a file. 
// It will send completion event to the sensor once data was processed. 
DataReceiver::DataReceiver(void)
{
	m_nProcessed = 0;
}

DataReceiver::~DataReceiver(void)
{
}

DataReceiver* DataReceiver::GetInstance()
{
	if (pReceiver== NULL) {
		pReceiver = new DataReceiver();
	}
	return pReceiver;
}

// Create event handlers and thread to synch 
// and expect data from sensors
bool DataReceiver::Start()
{
	DWORD dwThreadID;

	for (int i = 0; i < NUM_SENSORS; i++) 
    { 
        ghSentEvent[i] = CreateEvent( 
            NULL,   // default security attributes
            FALSE,  // auto-reset event object
            FALSE,  // initial state is nonsignaled
            NULL);  // unnamed object

		ghReceivedEvent[i] = CreateEvent( 
			NULL, 
			FALSE,
			FALSE,
			NULL);

        if (ghSentEvent[i] == NULL || ghReceivedEvent[i] == NULL) 
        { 
            cout << "CreateEvent failed " << GetLastError() << endl;
			return false;
        } 
    } 
	
	ghThread = CreateThread(
            NULL,              // default security
            0,                 // default stack size
            (LPTHREAD_START_ROUTINE)&ThreadProc,        // name of the thread function
            this,              // no thread parameters
            0,                // default startup flags
            &dwThreadID); 

	if (ghThread == NULL) 
	{
		cout << "CreateThread failed " << GetLastError() << endl;
		return false;
	}
	return true;
}

void DataReceiver::CloseEvents()
{
	for (int i = 0; i < NUM_SENSORS; i++) 
	{
		if (ghSentEvent[i] != NULL)
			CloseHandle(ghSentEvent[i]); 
	}
}

// This thread will wait for notification from sensors
// and invoke data receiving procedure
DWORD ThreadProc(LPVOID lpParam) 
{
	DataReceiver* pReceiverClass = (DataReceiver*) lpParam;
    DWORD dwWaitResult;

    //cout << "Thread " << GetCurrentThreadId() << " waiting for sent data event." << endl;
	bool bWait = true;
	while (bWait)
	{
		dwWaitResult = WaitForMultipleObjects( 
			NUM_SENSORS, // number of objects in array
			ghSentEvent, // array of objects
			FALSE,       // wait for any object
			10000);      // wait 10 seconds in our test. Otherwise use INFINITE for indefinite wait

		switch (dwWaitResult) 
		{
			// ghSentEvent[0] was signaled
		case WAIT_OBJECT_0 + 0: 
			if (!pReceiverClass->ReceiveData())
			{
				cout << "ReceiveData failed!" << endl;
				pReceiverClass->CloseEvents();
				return 0;
			}
			if (!pReceiverClass->SaveData(0))
			{
				cout << "SAVE ERROR!" << endl;
				pReceiverClass->CloseEvents();
				return 0;
			}
			break;

			// ghSentEvent[1] was signaled
		case WAIT_OBJECT_0 + 1: 
			if (!pReceiverClass->ReceiveData())
			{
				cout << "ReceiveData failed!" << endl;
				pReceiverClass->CloseEvents();
				return 0;
			}
			if (!pReceiverClass->SaveData(1))
			{
				cout << "SAVE ERROR!" << endl;
				pReceiverClass->CloseEvents();
				return 0;
			}
			//bWait = false;
			break; 

			// ghSentEvent[2] was signaled
		case WAIT_OBJECT_0 + 2: 
			if (!pReceiverClass->ReceiveData())
			{
				cout << "ReceiveData failed!" << endl;
				pReceiverClass->CloseEvents();
				return 0;
			}
			if (!pReceiverClass->SaveData(2))
			{
				cout << "SAVE ERROR!" << endl;
				pReceiverClass->CloseEvents();
				return 0;
			}
			bWait = false;
			break; 

		case WAIT_TIMEOUT:
			cout << "Wait for event timed out!" << endl;
			bWait = false;
			break;

			// An error occurred
		default: 
			cout << "Thread Wait error: " << GetLastError() << endl;
			pReceiverClass->CloseEvents();
			return 0; 
		}
	}

	pReceiverClass->CloseEvents();
	return 1;
}

// Connect to the named pipe to read/receive sensor data 
bool DataReceiver::ReceiveData()
{
	// Open the named pipe
    HANDLE pipe = CreateFile(
        L"\\\\.\\pipe\\minesense_pipe",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
 
    if (pipe == INVALID_HANDLE_VALUE) {
		cout << "DataReceiver: Failed to connect to pipe: " << GetLastError() << endl;
        return false;
    }
    //cout << "DataReceiver Reading data from pipe..." << endl;
 
    // The read operation will block until there is data to read
    wchar_t buffer[DATA_LENGTH+1];
    DWORD numBytesRead = 0;
    BOOL result = ReadFile(
        pipe,
        buffer, // the data from the pipe will be put here
        DATA_LENGTH * sizeof(wchar_t), // number of bytes allocated
        &numBytesRead, // this will store number of bytes actually read
        NULL // not using overlapped IO
    );
 
    if (!result) 
	{
		cout << "DataReceiver Failed to read data from the pipe." << endl;
		CloseHandle(pipe);
		return false;
	}
     
	buffer[numBytesRead / sizeof(wchar_t)] = '\0'; // null terminate the string
	//cout << "DataReceiver Number of bytes read: " << numBytesRead << endl;

	//convert from wide char to string
	char ch[DATA_LENGTH];
	char DefChar = ' ';
	int ret = WideCharToMultiByte(CP_ACP, 0, buffer, -1, ch, DATA_LENGTH, &DefChar, NULL);
	if (ret == 0)
	{
		cout << "Received data conversion WideCharToMultiByte failed" << GetLastError() << endl;
	}
	else 
	{
		m_sRcvData.append(ch);
		cout << "DataReceiver: " << numBytesRead << " bytes successfully received!" << endl;
		//cout << "DataReceiver data: " << m_sRcvData << endl;
	}

    CloseHandle(pipe);
	return true;
}

// Format data to user friendly form and write to a file.
bool DataReceiver::SaveData(int sensorIndex)
{
	const int expectedLines = 9;
	if (m_sRcvData.empty())
	{
		cout << "DataReceiver: Received data string is empty!" << endl;
		return false;
	}

	//parse m_sRcvData and separate lines
	vector<string> lines;
	istringstream ss(m_sRcvData);
	string token;
	while(std::getline(ss, token, ','))
	{
		lines.push_back(token);
		//cout << token << endl;
	}
	int numLines = (int) lines.size();
	if (numLines != expectedLines)
	{
		cout << "DataReceiver: Invalid number of lines received" << endl;
		return false;
	}

	ISensor::SensorType type = (ISensor::SensorType) atoi(lines[1].c_str()); 
	string sensorType = SensorTypeToString(type);
	ISensor::DataType metadatatype = (ISensor::DataType) atoi(lines[3].c_str());
	ISensor::DataUnit metadataunit = (ISensor::DataUnit) atoi(lines[4].c_str());
	ISensor::DataType supdatatype = (ISensor::DataType) atoi(lines[5].c_str());
	ISensor::DataUnit supdataunit = (ISensor::DataUnit) atoi(lines[6].c_str());

	string fileName = sensorType + "SensorReading.txt";
	ofstream dataFile(fileName);
	if (!dataFile.is_open())
	{
		cout << "Unable to open file" << endl;
		return false;
	}

	dataFile << "Version = " << lines[0] << endl << "Sensor Type = " << sensorType << endl << "Sensor Location = " << lines[2] << endl;
	dataFile << "DataType = " << DataTypeToString(metadatatype) << endl << "Data Units = " << DataUnitToString(metadataunit) << endl << endl;

	dataFile << "Supplementary Data Type = " <<  DataTypeToString(supdatatype) << endl << "Supplementary Data Units = " << DataUnitToString(supdataunit) << endl; 
	dataFile << "Timestamp = " << lines[7] << endl;
	dataFile << "Sensor Data = " << lines[8] << endl;
	
	dataFile.close();
	m_nProcessed++;
	m_sRcvData.clear();

	cout << "DataReceiver: SaveData success! Notify sensor..." << endl;
	// signal to server there is data to read
	if (!SetEvent(ghReceivedEvent[sensorIndex]) ) 
    {
        cout << "DataReceiver: SetEvent failed " << GetLastError() << endl;
		CloseHandle(ghReceivedEvent[sensorIndex]);
        return false;
    }

	return true;
}

// Returns number of data receive events processed
int DataReceiver::NumerOfSensorsProcessed() 
{
	return m_nProcessed;
}

//
// Following are Helper functions to convert enumeration values to the string
// 
string DataReceiver::SensorTypeToString(ISensor::SensorType type)
{
	map<ISensor::SensorType, string> types;
	types[ISensor::XRF] = "XRF";
	types[ISensor::EMS] = "EMS";
	types[ISensor::LIBS] = "LIBS";
	if(types.find(type) == types.end())
	{
		cout<<"Sensor type is not in the map!"<<endl;
		return "";
	}
	return types[type];
}

string DataReceiver::DataTypeToString(ISensor::DataType type)
{
	map<ISensor::DataType, string> types;
	types[ISensor::Temperature] = "Temperature";
	types[ISensor::Voltage] = "Voltage";	
	types[ISensor::Humidity] = "Humidity";
	types[ISensor::Frequency] = "Frequency";
	types[ISensor::Wavelength] = "Wavelength";
	types[ISensor::Energy] = "Energy";
	types[ISensor::Pressure] = "Pressure";
	if(types.find(type) == types.end())
	{
		cout<<"Data type is not in the map!"<<endl;
		return "";
	}
	return types[type];
}
string DataReceiver::DataUnitToString(ISensor::DataUnit unit)
{
	map<ISensor::DataUnit, string> types;
	types[ISensor::Celsius] = "Celsius";
	types[ISensor::Fahrenheit] = "Fahrenheit";
	types[ISensor::Volt] = "Volt";
	types[ISensor::EVolt] = "EVolt";
	types[ISensor::RH] = "RH";
	types[ISensor::Hz] = "Hz";
	types[ISensor::NM] = "NM";
	types[ISensor::Pa] = "Pa";
	if(types.find(unit) == types.end())
	{
		cout<<"Data unit is not in the map!"<<endl;
		return "";
	}
	return types[unit];
}

