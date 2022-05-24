
#include <src/COMToolkit2.h>
#include <iostream>

using namespace std;


void COMToolkit2::connect(int portNumber, int bR){
	if(status!=INIT)
		return; //Already connected!

	//Configure the conneciton
	std::cout << "Connecting to "<<portNumber << std::endl ;
	baudRate = bR;
	createFile(portNumber);
	setCommMask();
	defineControlSettings();
	setCommState();
	setTimeouts();
	
	if(anyErrors() == true){
		closeConnection();
		return;
	}
	else{
		status=READY;
		return;
	}
}


void COMToolkit2::sendByte(unsigned char byte){
	if(status!=READY)
		return; //Either the connection has not bee initialized or there have been errors...

	unsigned char buffer = 0;
	DWORD numOfBytesWritten = 0;
	buffer = (unsigned char)byte ;
	if(!WriteFile(hSerial, &buffer, sizeof(buffer), &numOfBytesWritten, NULL)){
		;//cout << "error writing" << endl ;
	}
	/*else{
		//printf("Sent %d\n", byte);
		//cout << "Sent: " << buffer << endl ;
	}*/
	SetFilePointer(hSerial,0,0,FILE_BEGIN);
}



void COMToolkit2::sendString(unsigned char* buffer, int numBytes){
	if(status!=READY)
		return; //Either the connection has not bee initialized or there have been errors...

	DWORD numOfBytesWritten = 0;
	if(!WriteFile(hSerial, buffer, numBytes, &numOfBytesWritten, NULL)){
		std::cout << "error writing" << endl ;
	}
	/*else{
		std::cout << "Sent: " << buffer << endl ;
	}*/
	SetFilePointer(hSerial,0,0,FILE_BEGIN);
}

bool COMToolkit2::anyErrors(){
	if(status == ERRORS){
		return true;
	}
	else{
		cout << "Connection successful." << endl ;
		return false;
	}
}

void COMToolkit2::closeConnection(){

	if(status==INIT)
		return; //The connection has not been stablished yet!
	
	//Close connection
	if(CloseHandle(hSerial)){
		cout << "COM port closed" << endl;
	}
	else{
		cout << "Error closing COM port" << endl;
	}
}


//UTILITY METHODS

void COMToolkit2::createFile(int portNumber){
	wchar_t portW[12];
	std::swprintf(portW, 12,L"//./COM%d", portNumber);
	hSerial = CreateFileW(portW,
						GENERIC_READ | GENERIC_WRITE,
						0, 
						0,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,//|FILE_FLAG_OVERLAPPED
						0);

    if(hSerial==INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND)
        {
			//serial port does not exist. Inform user.
			cout << "The port does not exsist" << endl ;
			status = ERRORS;
        }

		//some other error occurred. Inform user.
		cout << "Invalid handle value" << endl ;
		status = ERRORS;
    }
}

void COMToolkit2::setCommMask(){
	if (!SetCommMask(hSerial, EV_RXCHAR)){
		// Error setting communications event mask.
		cout << "Error setting communications event mask" << endl ;
		status = ERRORS;
	}
}

void COMToolkit2::defineControlSettings(){
	FillMemory(&dcbSerialParams, sizeof(dcbSerialParams), 0);
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	if (!GetCommState(hSerial, &dcbSerialParams)) {
		//error getting state
		cout << "Error getting state" << endl ;
		status = ERRORS;
	}
	//Configuring the serial port: http://msdn.microsoft.com/en-us/library/aa450503.aspx
	dcbSerialParams.BaudRate=baudRate ;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.fBinary=TRUE;				//It has to be binary
	dcbSerialParams.Parity=NOPARITY;			//No parity checking
	dcbSerialParams.fOutxCtsFlow=FALSE;			//No flow control
	dcbSerialParams.fOutxDsrFlow=FALSE;
	//dcbSerialParams.fDtrControl=DTR_CONTROL_DISABLE;
	dcbSerialParams.fTXContinueOnXoff = TRUE;     // XOFF continues Tx 
	dcbSerialParams.fOutX = FALSE;                // No XON/XOFF out flow control 
	dcbSerialParams.fInX = FALSE;                 // No XON/XOFF in flow control 
	dcbSerialParams.fErrorChar = FALSE;           // Disable error replacement 
	dcbSerialParams.fNull = FALSE;                // Disable null stripping 
	dcbSerialParams.fDsrSensitivity=FALSE;		//Receive ANY incomming bytes
	dcbSerialParams.StopBits=ONESTOPBIT;
	//dcbSerialParams.fRtsControl=RTS_CONTROL_DISABLE;
	
	//dcbSerialParams.BaudRate=CBR_57600;
	//dcbSerialParams.BaudRate=CBR_115200;
	//dcbSerialParams.BaudRate=345600;
	//dcbSerialParams.BaudRate=4*115200;
	//dcbSerialParams.BaudRate=57600;

	//dcbSerialParams.BaudRate=9600;
	//dcbSerialParams.BaudRate=230400;
	//dcbSerialParams.ByteSize=8;
	//dcbSerialParams.StopBits=ONESTOPBIT;
	//dcbSerialParams.Parity=NOPARITY;
}

void COMToolkit2::setCommState(){
	if(!SetCommState(hSerial, &dcbSerialParams)){
		//error setting serial port state
		cout << "Error setting serial port state" << endl ;
		status = ERRORS;
	}
}

void COMToolkit2::setTimeouts(){
	// From MSDN: Specifies the maximum acceptable time, in milliseconds, to elapse between the arrival of two characters on the communication line.
    timeouts.ReadIntervalTimeout=1; //50 //MAXWORD
	// From MSDN: Specifies the constant, in milliseconds, used to calculate the total timeout period for read operations.
    timeouts.ReadTotalTimeoutConstant=1; //50 //0
	// From MSDN: Specifies the multiplier, in milliseconds, used to calculate the total timeout period for read operations.
    timeouts.ReadTotalTimeoutMultiplier=1; //10 //0
	// From MSDN: Specifies the constant, in milliseconds, used to calculate the total timeout period for write operations.
    timeouts.WriteTotalTimeoutConstant=1; //50 //0
	// From MSDN: Specifies the multiplier, in milliseconds, used to calculate the total timeout period for write operations.
    timeouts.WriteTotalTimeoutMultiplier=1; //10 //0
	if(!SetCommTimeouts(hSerial, &timeouts)){
        //error occureed. Inform user
		cout << "Timeout error occurred." << endl ;
		status = ERRORS;
    }
}

void COMToolkit2::setNoTimeouts(){
	// From MSDN: Specifies the maximum acceptable time, in milliseconds, to elapse between the arrival of two characters on the communication line.
    timeouts.ReadIntervalTimeout=MAXDWORD; //50 //MAXWORD
	// From MSDN: Specifies the constant, in milliseconds, used to calculate the total timeout period for read operations.
    timeouts.ReadTotalTimeoutConstant=0; //50 //0
	// From MSDN: Specifies the multiplier, in milliseconds, used to calculate the total timeout period for read operations.
    timeouts.ReadTotalTimeoutMultiplier=0; //10 //0
	// From MSDN: Specifies the constant, in milliseconds, used to calculate the total timeout period for write operations.
    timeouts.WriteTotalTimeoutConstant=1; //50 //0
	// From MSDN: Specifies the multiplier, in milliseconds, used to calculate the total timeout period for write operations.
    timeouts.WriteTotalTimeoutMultiplier=1; //10 //0
	if(!SetCommTimeouts(hSerial, &timeouts)){
        //error occureed. Inform user
		cout << "Timeout error occurred." << endl ;
		status = ERRORS;
    }
}

void COMToolkit2::uSleep(int waitTime) {
    __int64 time1 = 0, time2 = 0, freq = 0;

    QueryPerformanceCounter((LARGE_INTEGER *) &time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    do {
        QueryPerformanceCounter((LARGE_INTEGER *) &time2);
    } while((time2-time1) < waitTime);
}