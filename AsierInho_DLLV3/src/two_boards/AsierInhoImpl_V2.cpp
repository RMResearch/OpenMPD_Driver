#include <src/AsierInhoImpl_V2.h>
#include <../AsierInho_DLL/src/ParseBoardConfig.h>
#include <sstream>
#include <fstream>

static char consoleLineBuffer[512];

/**
	Method run by each of the wroker threads
*/
void* worker_BoardUpdater(void* threadData) {
	//we read ou configuration
	struct worker_threadData* data = (struct worker_threadData*)threadData;
	
	while (data->running) {
		//Wait for our signal
		pthread_mutex_lock(&data->send_signal);
		//Send update: 		
		AsierInhoImpl_V2::_sendUpdate(data->board, data->dataStream, 1);
		//Send notification signal
		pthread_mutex_unlock(&data->done_signal);		
	}	
	AsierInho_V2::printMessage("AsierInho Worker finished!\n");
	return 0;
}


AsierInhoImpl_V2::AsierInhoImpl_V2() :AsierInhoBoard_V2(), status(INIT) {
	//Initialise configuration parameters for each thread: 
	topData = new struct worker_threadData;
	bottomData = new struct worker_threadData;
	bottomData->dataStream = &(this->dataStream[0]);
	topData->dataStream = &(this->dataStream[512]);	
}

AsierInhoImpl_V2::~AsierInhoImpl_V2() {
	disconnect();
	delete topData;
	delete bottomData;
}

/**
	Connects to the board, using the board type and ID specified.
	ID corresponds to the index we labeled on each board.
	Returns true if connection was succesfull.
*/
bool AsierInhoImpl_V2::connect(int bottomBoardID, int topBoardID) {
	this->topBoardID = topBoardID;
	this->botBoardID = bottomBoardID;
	//setBoardConfig(boardType, bottomBoardID, topBoardID);
	float transducerPositions[512 * 3], amplitudeAdjust[512];
	int phaseAdjust[512], transducerIds[512];
	int numDiscreteLevels;
	readParameters(transducerPositions, transducerIds, phaseAdjust, amplitudeAdjust, &numDiscreteLevels);
	//Store in local variables (to be deprecated):
	//a. As independent boards (this should get deprecated)
	{
		this->phaseAdjustBottom = new int[256];
		this->phaseAdjustTop = new int[256];
		memcpy(phaseAdjustBottom, phaseAdjust, 256 * sizeof(int));
		memcpy(phaseAdjustTop, &(phaseAdjust[256]), 256 * sizeof(int));
		memcpy(this->phaseAdjust, phaseAdjust, 512 * sizeof(int));
		memcpy(this->transducerIds, transducerIds, 512 * sizeof(int));
		for (int t = 256; t < 512; t++)this->transducerIds[t] += 256;
	}
	
	if (status == AsierInhoState::INIT) {
		if (initFTDevice((bottomData->board), bottomSerialNumber) && initFTDevice((topData->board), topSerialNumber)) { // open via the serial number of USB board, not COM port
			//printf("Connection success!\n");
			status = AsierInhoState::CONNECTED;
			//Initialise signals
			pthread_mutex_init(&(topData->send_signal), NULL);
			pthread_mutex_lock(&(topData->send_signal));
			pthread_mutex_init(&(bottomData->send_signal), NULL);
			pthread_mutex_lock(&(bottomData->send_signal));
			pthread_mutex_init(&(topData->done_signal), NULL);
			pthread_mutex_init(&(bottomData->done_signal), NULL);
			//Create threads with their initialization parameters: 
			pthread_attr_t attrs;
			pthread_attr_init(&attrs);
			pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
			pthread_create(&topBoardThread, &attrs 	, &worker_BoardUpdater, ((void*)topData));
			pthread_create(&bottomBoardThread, &attrs 	, &worker_BoardUpdater, ((void*)bottomData));
			Sleep(100);
			AsierInho_V2::printMessage("AsierInho_V2 connected succesfully\n");
			return true;
		}
		else {
			AsierInho_V2::printWarning("AsierInho_V2 connection failed\n");
			return false;
		}
	}
	AsierInho_V2::printWarning("AsierInho_V2 is already connected\n");
	return false;
}

void AsierInhoImpl_V2::readParameters(float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels) {
	BoardConfig topConfig= ParseBoardConfig::readParameters(topBoardID);
	BoardConfig botConfig= ParseBoardConfig::readParameters(botBoardID);
	this->bottomSerialNumber = botConfig.hardwareID;
	this->topSerialNumber = topConfig.hardwareID;
	float matTopToWorld[16] = {-1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0,-1, 0.2388f,
								0, 0, 0, 1,	
	};
	float matBotToWorld[16] = {	1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1,	
	};
	//0. Num discrete levels
	*numDiscreteLevels = topConfig.numDiscreteLevels;

	//1. Write transducer positions: 
	//1.a. Bottom board: 
	for (int t = 0; t < 256; t++) {
		float* pLocal = &(botConfig.positions[3 * t]);
		float posWorld[3] = {pLocal[0]*matBotToWorld[0] + pLocal[1]*matBotToWorld[1] + pLocal[2]*matBotToWorld[2] + matBotToWorld[3],
							 pLocal[0]*matBotToWorld[4] + pLocal[1]*matBotToWorld[5] + pLocal[2]*matBotToWorld[6] + matBotToWorld[7],
							 pLocal[0]*matBotToWorld[8] + pLocal[1]*matBotToWorld[9] + pLocal[2]*matBotToWorld[10] + matBotToWorld[11],};
		memcpy(&(transducerPositions[3 * t]), posWorld, 3 * sizeof(float));
	}
	//1.b. Bottom board: 
	for (int t = 0; t < 256; t++) {
		float* pLocal = &(topConfig.positions[3 * t]);
		float posWorld[3] = {pLocal[0]*matTopToWorld[0] + pLocal[1]*matTopToWorld[1] + pLocal[2]*matTopToWorld[2] + matTopToWorld[3],
							 pLocal[0]*matTopToWorld[4] + pLocal[1]*matTopToWorld[5] + pLocal[2]*matTopToWorld[6] + matTopToWorld[7],
							 pLocal[0]*matTopToWorld[8] + pLocal[1]*matTopToWorld[9] + pLocal[2]*matTopToWorld[10] + matTopToWorld[11],};
		memcpy(&(transducerPositions[3 * (t+256)]), posWorld, 3 * sizeof(float));
	}
	//2. Write transducer IDs:
	memcpy(transducerIds, botConfig.pinMapping, 256 * sizeof(int));
	memcpy(&(transducerIds[256]), topConfig.pinMapping, 256 * sizeof(int));
	//3. Write Phase adjustments:
	memcpy(phaseAdjust, botConfig.phaseAdjust, 256 * sizeof(int));
	memcpy(&(phaseAdjust[256]), topConfig.phaseAdjust, 256 * sizeof(int));
	//4. Write Amplitude adjustments:
	memcpy(amplitudeAdjust, botConfig.amplitudeAdjust, 256 * sizeof(float));
	memcpy(&(amplitudeAdjust[256]), topConfig.amplitudeAdjust, 256 * sizeof(float));

}


// To achieve 10kHz, I need to use the FTD2XX driver. USBs are not recognised as a COM port any more...
bool AsierInhoImpl_V2::initFTDevice(FT_HANDLE& fthandle, char* serialNumber, bool syncMode) {

	FT_STATUS status;
	status = FT_OpenEx(serialNumber, FT_OPEN_BY_SERIAL_NUMBER, &fthandle);
	if (status != FT_OK) {
		if (status == FT_INVALID_HANDLE)	sprintf_s<512>(consoleLineBuffer, "Could not open USB port, status not ok (%d - invalid handle...)\n", status);
		if (status == FT_DEVICE_NOT_FOUND)	sprintf_s<512>(consoleLineBuffer, "Could not open USB port, open status not ok (%d - device not found...)\n", status);
		if (status == FT_DEVICE_NOT_OPENED) sprintf_s<512>(consoleLineBuffer, "Could not open USB port, open status not ok (%d - device not opened...)\n", status);
		AsierInho_V2::printWarning(consoleLineBuffer);
		return false;
	}
	AsierInho_V2::printMessage("USB ports open\n");
	status = FT_ResetDevice(fthandle);
	if (status != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "Could not reset USB ports. Reset status %d\n", status);
		AsierInho_V2::printWarning(consoleLineBuffer);
		return false;
	}

	// set the transfer mode as Syncronsou mode
	if (syncMode) {
		UCHAR Mask = 0xFF; // Set data bus to outputs
		UCHAR mode_rst = 0;
		UCHAR mode_sync = 0x40; // Configure FT2232H into 0x40 Sync FIFO mode
		FT_SetBitMode(fthandle, Mask, mode_rst); // reset MPSSE
		FT_SetBitMode(fthandle, Mask, mode_sync); // configure FT2232H into Sync FIFO mode
	}

	// set the latency timer and USB parameters
	FT_SetLatencyTimer(fthandle, 2);
	FT_SetUSBParameters(fthandle, 0x10000, 0x10000);
	FT_Purge(fthandle, FT_PURGE_RX | FT_PURGE_TX);
	AsierInho_V2::printMessage("USB ports setup\n");
	return true;
}

void AsierInhoImpl_V2::updateMessage(unsigned char* message) {
	//0. Check status
	if (status != AsierInhoState::CONNECTED )
		return;
	//Multithreaded version: 
	//Wait for them to finish
	pthread_mutex_lock(&(topData->done_signal));
	pthread_mutex_lock(&(bottomData->done_signal));
	//Update the buffer:
	memcpy(dataStream, message, messageSize*numBoards);
	//Send signals to notify worker threads to update phases
	pthread_mutex_unlock(&(topData->send_signal));
	pthread_mutex_unlock(&(bottomData->send_signal));
}

/**
	This method turns the transducers off (so that the board does not heat up/die misserably)
	The board is still connected, so it can later be used again (e.g. create new traps)
*/
void AsierInhoImpl_V2::turnTransducersOff() {
	//0. Check status
	if (status != AsierInhoState::CONNECTED)
		return;

	//1. Set all phases and amplitudes to "0"== OFF and send!
	static unsigned char message[1024];
	memset(message, 0, 1024* sizeof(unsigned char));	//Set all phases and amplitudes.
	message[0] = message[512] = 0 + 128;	//Set flag indicating new update (>128)
	message[25] = message[512+25] = 128;		//Set flag indicating to change the LED colour (turn off in this case)
	//2. Send it: 
	updateMessage(message);
}

void AsierInhoImpl_V2::turnTransducersOn() {
	//1. Set all phases and amplitudes to "0"== OFF and send!
	static unsigned char message[1024];
	memset(message, 64, 1024 * sizeof(unsigned char));	//Set all phases and amplitudes.
	message[0] = message[512] = 64 + 128;	//Set flag indicating new update (>128)
	//2. Send it: 
	updateMessage(message);
}

/**
	This method disconnects the board, closing COM ports.
*/
void AsierInhoImpl_V2::disconnect() {
	if (status != AsierInhoState::CONNECTED)
		return;
	//0. Notify threads to end and wait for them
	topData->running = false;
	bottomData->running = false;
	pthread_mutex_unlock(&topData->send_signal);
	pthread_mutex_unlock(&bottomData->send_signal);
	pthread_join(topBoardThread, NULL);
	pthread_join(bottomBoardThread, NULL);
	//1. Destroy threads and related resources (e.g. signals)
	pthread_mutex_destroy(&(topData->send_signal));
	pthread_mutex_destroy(&(topData->done_signal));
	//pthread_mutex_destroy(&(bottomData->send_signal));
	pthread_mutex_destroy(&(bottomData->done_signal));
	//2. Clos ports:
	FT_Close(topData->board);
	FT_Close(bottomData->board);
	status = AsierInhoState::INIT;
}


/**
	Sends a command to the board to load new phases and amplitudes for its 256 transducers.
*/
void AsierInhoImpl_V2::_sendUpdate(FT_HANDLE& board, unsigned char* stream, size_t numMessages) {

	FT_STATUS ftStatus; DWORD dataWritten;
	//4. Send!
	ftStatus = FT_Write(board, stream, (DWORD)numMessages*AsierInhoImpl_V2::messageSize, &dataWritten);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl_V2::_sendUpdate error %d\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
}

