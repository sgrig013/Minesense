#include "StdAfx.h"
#include "Sensors.h"
using namespace std;
using namespace constants;

extern HANDLE ghSentEvent[NUM_SENSORS];
extern HANDLE ghReceivedEvent[NUM_SENSORS];

static bool SendToReceiver(int sensorIndex, string data);
static bool WaitForCompletionEvent(int sensorIndex);

SensorXRF::SensorXRF(void)
{
}

SensorXRF::~SensorXRF(void)
{
}

bool SensorXRF::Start()
{
	string xrf_data = GenerateData();

	return SendData(xrf_data);
}

string SensorXRF::GenerateData()
{
	stringstream ss;
	SensorMetaData metaData;
	metaData.version = "1.0.1";
	metaData.sensor = XRF;
	metaData.sensorLocation = "location 1";
	metaData.metric = Pressure;
	metaData.unit = Pa; 

	ss << metaData.version << "," << metaData.sensor << "," << metaData.sensorLocation << "," << metaData.metric << "," << metaData.unit << ",";

	SensorReading sensorData;
	sensorData.sup_data = Wavelength;
	sensorData.sup_unit = NM;
	sensorData.timestamp = GetLocalTime();
	sensorData.data = GetRandomNumericData(30);

	ss << sensorData.sup_data << "," << sensorData.sup_unit << "," << sensorData.timestamp << "," << sensorData.data;
	string data = ss.str();

	return data;
}

bool SensorXRF::SendData(string data)
{
	bool result = SendToReceiver(0, data);
	if (result)
	{
		// wait until receiver notification to exit
		if (WaitForCompletionEvent(0))
		{
			cout << "Sensor XRF: TRANSFER COMPLETE!\n" << endl;
		}
	}
	return result;
}



//
// EMS type sensor specific class
//
SensorEMS::SensorEMS(void)
{
}

SensorEMS::~SensorEMS(void)
{
}

bool SensorEMS::Start()
{
	string ems_data = GenerateData();
	return  SendData(ems_data);
}

string SensorEMS::GenerateData()
{
	stringstream ss;
	SensorMetaData metaData;
	metaData.version = "1.0.2";
	metaData.sensor = EMS;
	metaData.sensorLocation = "location 2";
	metaData.metric = Frequency;
	metaData.unit = Hz; 

	ss << metaData.version << "," << metaData.sensor << "," << metaData.sensorLocation << "," << metaData.metric << "," << metaData.unit << ",";

	SensorReading sensorData;
	sensorData.sup_data = Voltage;
	sensorData.sup_unit = Volt;
	sensorData.timestamp = GetLocalTime();
	sensorData.data = GetRandomNumericData(40);

	ss << sensorData.sup_data << "," << sensorData.sup_unit << "," << sensorData.timestamp << "," << sensorData.data;
	string data = ss.str();

	return data;
}

bool SensorEMS::SendData(string data)
{
	bool result = SendToReceiver(1, data);
	if (result)
	{
		// wait until receiver notification to exit
		if (WaitForCompletionEvent(1))
		{
			cout << "Sensor EMS: TRANSFER COMPLETE!\n" << endl;
		}
	}
	return result;
}


//
// LIBS type sensor specific class
//
SensorLIBS::SensorLIBS(void)
{
}

SensorLIBS::~SensorLIBS(void)
{
}

bool SensorLIBS::Start()
{
	string libs_data = GenerateData();
	bool result = SendData(libs_data);
	
	return result;
}

string SensorLIBS::GenerateData()
{
	stringstream ss;
	SensorMetaData metaData;
	metaData.version = "1.0.3";
	metaData.sensor = LIBS;
	metaData.sensorLocation = "location 3";
	metaData.metric = Humidity;
	metaData.unit = RH; 

	ss << metaData.version << "," << metaData.sensor << "," << metaData.sensorLocation << "," << metaData.metric << "," << metaData.unit << ",";

	SensorReading sensorData;
	sensorData.sup_data = Temperature;
	sensorData.sup_unit = Celsius;
	sensorData.timestamp = GetLocalTime();
	sensorData.data = GetRandomNumericData(50);

	ss << sensorData.sup_data << "," << sensorData.sup_unit << "," << sensorData.timestamp << "," << sensorData.data;
	string data = ss.str();

	return data;
}

bool SensorLIBS::SendData(string data)
{
	bool result = SendToReceiver(2, data);
	if (result)
	{
		// wait until receiver notification to exit
		if (WaitForCompletionEvent(2))
		{
			cout << "Sensor LIBS: TRANSFER COMPLETE!\n" << endl;
		}
	}
	return result;
}

//
// global methods to use by sensor classes
//
static bool SendToReceiver(int sensorIndex, string data)
{
	// Create a pipe to send data
    HANDLE pipe = CreateNamedPipe(
        L"\\\\.\\pipe\\minesense_pipe", // name of the pipe
        PIPE_ACCESS_OUTBOUND, // 1-way pipe -- send only
        PIPE_TYPE_BYTE, // send data as a byte stream
        3, // only allow 1 instance of this pipe
        0, // no outbound buffer
        0, // no inbound buffer
        0, // use default wait time
        NULL // use default security attributes
    );
 
    if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
        cout << "Sensor Failed to create outbound pipe instance." << GetLastError() << endl;

        return false;
    }
	
    cout << "Sensor Waiting for a client to connect to the pipe..." << endl;
 
	// signal to server there is data to read
	if (!SetEvent(ghSentEvent[sensorIndex]) ) 
    {
        cout << "Sensor SetEvent failed " << GetLastError() << endl;
		CloseHandle(pipe);
        return false;
    }
    // This call blocks until a client process connects to the pipe
    BOOL result = ConnectNamedPipe(pipe, NULL);
    if (!result) 
	{
        cout << "Sensor Failed to make connection on named pipe." << GetLastError() << endl;

		CloseHandle(pipe);
        return false;
    }

	//convert sensor data to wide string
	wstring wsData = wstring(data.begin(), data.end());
	const wchar_t* pwsData = wsData.c_str();
    cout << "Sensor writing data to pipe..." << endl;
 
    // This call blocks until a client process reads all the data
    DWORD numBytesWritten = 0;
    result = WriteFile(
        pipe,		// handle to our outbound pipe
        pwsData,	// data to send
        wcslen(pwsData) * sizeof(wchar_t), // length of data to send (bytes)
        &numBytesWritten, // will store actual amount of data sent
        NULL		// not using overlapped IO
    );
    if (!result) 
	{
		cout << "Sensor Failed to send data." << GetLastError() << endl;
		CloseHandle(pipe);
		return false;
    } 
    cout << "Sensor Number of bytes sent: " << numBytesWritten << endl;

	DisconnectNamedPipe(pipe);
	CloseHandle(pipe);
	
	return true;
}

static bool WaitForCompletionEvent(int sensorIndex)
{
	DWORD dwWaitResult;
    //cout << "Sensor waiting for data received event." << endl;
    
    dwWaitResult = WaitForSingleObject( 
        ghReceivedEvent[sensorIndex], // event handle
        5000);			 // wait 5 seconds   

    switch (dwWaitResult) 
    {
        case WAIT_OBJECT_0: 
            //cout << "Sensor: TRANSFER COMPLETE event!" << endl;
			if (ghReceivedEvent != NULL)
				CloseHandle(ghReceivedEvent[sensorIndex]);
            break;

        // An error occurred
        default: 
            cout << "Sensor event wait error: " << GetLastError() << endl;
            return false; 
    }
	return true;
}

