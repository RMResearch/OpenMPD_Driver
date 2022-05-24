#include <src/AsierInhoImpl_V3.h>
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
		while(!data->send_signal);
		data->send_signal = false;
		//Send update: 		
		AsierInhoImpl_V3::_sendUpdate(data->board, data->dataStream, 1);
#ifdef _TIME_PROFILING
		struct timeval curTime;
		data->curUpdate = (data->curUpdate + 1) % data->UPS;
		gettimeofday(&curTime, 0x0);
		data->timestamps[data->curUpdate] = computeTimeElapsed(data->referenceTime, curTime);
#endif
		//Send notification signal
		pthread_mutex_unlock(&data->done_signal);		
	}	
	AsierInho_V3::printMessage("AsierInho Worker finished!\n");
	return 0;
}


#ifdef _TIME_PROFILING
void AsierInhoImpl_V3::_profileTimes() {
	//Lock all boards:
	std::vector<bool> send_signal_state;
	for (int b = 0; b < boardWorkerData.size(); b++) {
		//pthread_mutex_lock(&(boardWorkerData[b]->send_signal));
		send_signal_state.push_back(boardWorkerData[b]->send_signal);
		boardWorkerData[b]->send_signal = false;
	}
	//Write all time profiles: 
	for (int b = 0; b < boardWorkerData.size(); b++) {
		FILE* boardFile;
		fopen_s(&boardFile, boardWorkerData[b]->boardSN, "a");
		for (int u = 0; u < boardWorkerData[b]->UPS; u++) {
			int update = (u + boardWorkerData[b]->curUpdate) % boardWorkerData[b]->UPS;
			fprintf_s(boardFile, "%d, %f\n", update, boardWorkerData[b]->timestamps[update]);
		}
		fclose(boardFile);
	}
	//Restore all boards:
	for (int b = 0; b < boardWorkerData.size(); b++) {
		boardWorkerData[b]->send_signal=send_signal_state[b];
	}
	
}

#endif

AsierInhoImpl_V3::AsierInhoImpl_V3() :AsierInhoBoard_V3(), status(INIT) {
	
}

AsierInhoImpl_V3::~AsierInhoImpl_V3() {
	disconnect();
	//delete topData;
	//delete bottomData;
	for (int b = 0; b < boardWorkerData.size(); b++)
		delete boardWorkerData[b];
	boardWorkerData.clear();
}

/**
	Connects to the board, using the board type and ID specified.
	ID corresponds to the index we labeled on each board.
	Returns true if connection was succesfull.
*/
bool AsierInhoImpl_V3::connect(int bottomBoardID, int topBoardID) {
	//Configuration data for a simple top-bottom setup.
	int boardIDs[] = { bottomBoardID, topBoardID };
	float matBoardToWorld[32] = {	/*bottom*/
								1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1,	
								/*top*/
								-1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0,-1, 0.2388f,
								0, 0, 0, 1,	
	};
	return connect(2, boardIDs, matBoardToWorld);
}

bool AsierInhoImpl_V3::connect(int numBoards, int * boardIDs, float* matToWorld)
{
	this->numBoards = numBoards;
	//1. Create resources for worker threads: 
	dataStream = new unsigned char[messageSize*numBoards];
	transducerPositions = new float[256 * 3 * numBoards];
	amplitudeAdjust= new float[256 * numBoards];
	phaseAdjust = new int[256 * numBoards];
	transducerIds = new int [256*numBoards];
	int* numDiscreteLevels = new int [numBoards];
	this->numDiscreteLevels = 256; //We will set the resolution to the MINIMUM of all board involved
	//2. Read per-board adjustment parameters and safe them in each worker thread data:
#ifdef _TIME_PROFILING
	struct timeval reference;
	gettimeofday(&reference, 0x0);
#endif
	for (int b = 0; b < numBoards; b++) {
		this->boardIDs.push_back(boardIDs[b]);	
		boardWorkerData.push_back(new struct worker_threadData);
		boardWorkerData[b]->dataStream = &(dataStream[b*messageSize]);
		readBoardParameters(boardIDs[b], &(matToWorld[16*b]), &(transducerPositions[3*256*b]), &(transducerIds[256*b]), &(phaseAdjust[256*b]), &(amplitudeAdjust[256*b]), &numDiscreteLevels[b], (boardWorkerData[b]));
		if (numDiscreteLevels[b] < this->numDiscreteLevels)
			this->numDiscreteLevels = numDiscreteLevels[b];
#ifdef _TIME_PROFILING		//DEBUG: Add time profiling fields:
		boardWorkerData[b]->referenceTime = reference;
		boardWorkerData[b]->curUpdate = 0;
#endif 	//END DEBUG

	}
	delete numDiscreteLevels;

	//Adjust from (local) transducer IDs [0..255] to global positions in the message 
	{
		for (int b = 1; b < numBoards; b++)
			for (int t = 0; t < 256; t++)
				this->transducerIds[t + b * 256] += b * 256;
	}
	//3. Connect to each of the boards:
	if (status == AsierInhoState::INIT) {
		bool allConnected=true;
		int boardsConnected;
		for (boardsConnected = 0; boardsConnected < numBoards && allConnected; boardsConnected++) {
			allConnected &= initFTDevice((boardWorkerData[boardsConnected]->board), boardWorkerData[boardsConnected]->boardSN, false);
		}

		if (allConnected){
			status = AsierInhoState::CONNECTED;
			for (int b = 0; b < numBoards; b++) {
				//pthread_mutex_init(&(boardWorkerData[b]->send_signal), NULL);
				//pthread_mutex_lock(&(boardWorkerData[b]->send_signal));
				pthread_mutex_init(&(boardWorkerData[b]->done_signal), NULL);
				pthread_attr_t attrs;
				pthread_attr_init(&attrs);
				pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
				pthread_t thread; 
				boardWorkerThreads.push_back(thread);
				pthread_create(&boardWorkerThreads[b], &attrs 	, &worker_BoardUpdater, ((void*)boardWorkerData[b]));
			
			}			
			Sleep(100);
			AsierInho_V3::printMessage("AsierInho_V3 connected succesfully\n");
			return true;
		}
		else {
			//Destroy connections created? (0..boardConnected)
			AsierInho_V3::printWarning("AsierInho_V3 connection failed\n");
			return false;
		}
	}
	AsierInho_V3::printWarning("AsierInho_V3 is already connected\n");
	return false;
}

void AsierInhoImpl_V3::readParameters(float * transducerPositions, int * transducerIds, int * phaseAdjust, float * amplitudeAdjust, int * numDiscreteLevels)
{
	memcpy(transducerPositions, this->transducerPositions, 256*numBoards*3*sizeof(float));
	memcpy(transducerIds , this->transducerIds, 256*numBoards*sizeof(int));
	memcpy(phaseAdjust , this->phaseAdjust, 256*numBoards*sizeof(int));
	memcpy(amplitudeAdjust , this->amplitudeAdjust, 256*numBoards*sizeof(float));
	*numDiscreteLevels = this->numDiscreteLevels;
}

void AsierInhoImpl_V3::readBoardParameters(int boardId, float* matToWorld, float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels, worker_threadData* thread_data) {
	BoardConfig boardConfig= ParseBoardConfig::readParameters(boardId);
	thread_data->boardSN = new char[strlen(boardConfig.hardwareID) +1];
	strcpy_s(thread_data->boardSN, strlen(boardConfig.hardwareID) + 1, boardConfig.hardwareID);
	//0. Num discrete levels
	*numDiscreteLevels = boardConfig.numDiscreteLevels;
	//1. Write transducer positions (multiply local pos by matrix): 
	for (int t = 0; t < 256; t++) {
		float* pLocal = &(boardConfig.positions[3 * t]);
		float posWorld[3] = {pLocal[0]*matToWorld[0] + pLocal[1]*matToWorld[1] + pLocal[2]*matToWorld[2] + matToWorld[3],
							 pLocal[0]*matToWorld[4] + pLocal[1]*matToWorld[5] + pLocal[2]*matToWorld[6] + matToWorld[7],
							 pLocal[0]*matToWorld[8] + pLocal[1]*matToWorld[9] + pLocal[2]*matToWorld[10] + matToWorld[11],};
		memcpy(&(transducerPositions[3 * t]), posWorld, 3 * sizeof(float));
	}	
	//2. Write transducer IDs:
	memcpy(transducerIds, boardConfig.pinMapping, 256 * sizeof(int));
	//3. Write Phase adjustments:
	memcpy(phaseAdjust, boardConfig.phaseAdjust, 256 * sizeof(int));
	//4. Write Amplitude adjustments:
	memcpy(amplitudeAdjust, boardConfig.amplitudeAdjust, 256 * sizeof(float));

}

void readParameters(float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels) {
	;
}

// To achieve 10kHz, I need to use the FTD2XX driver. USBs are not recognised as a COM port any more...
bool AsierInhoImpl_V3::initFTDevice(FT_HANDLE& fthandle, char* serialNumber, bool syncMode) {

	FT_STATUS status;
	status = FT_OpenEx(serialNumber, FT_OPEN_BY_SERIAL_NUMBER, &fthandle);
	if (status != FT_OK) {
		if (status == FT_INVALID_HANDLE)	sprintf_s<512>(consoleLineBuffer, "Could not open USB port, status not ok (%d - invalid handle...)\n", status);
		if (status == FT_DEVICE_NOT_FOUND)	sprintf_s<512>(consoleLineBuffer, "Could not open USB port, open status not ok (%d - device not found...)\n", status);
		if (status == FT_DEVICE_NOT_OPENED) sprintf_s<512>(consoleLineBuffer, "Could not open USB port, open status not ok (%d - device not opened...)\n", status);
		AsierInho_V3::printWarning(consoleLineBuffer);
		return false;
	}
	AsierInho_V3::printMessage("USB ports open\n");
	status = FT_ResetDevice(fthandle);
	if (status != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "Could not reset USB ports. Reset status %d\n", status);
		AsierInho_V3::printWarning(consoleLineBuffer);
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
	AsierInho_V3::printMessage("USB ports setup\n");
	return true;
}

void AsierInhoImpl_V3::updateMessage(unsigned char* message) {
	//0. Check status
	if (status != AsierInhoState::CONNECTED )
		return;
	//Multithreaded version: 
	//Wait for them to finish
	for (int b = 0; b < numBoards; b++) {
		pthread_mutex_lock(&(boardWorkerData[b]->done_signal));
		//boardWorkerData[b]->dataStream=message;
	}
	
	//Update the buffer:
	memcpy(dataStream, message, messageSize*numBoards);
	//Send signals to notify worker threads to update phases
	for (int b = 0; b < numBoards; b++)
		//pthread_mutex_unlock(&(boardWorkerData[b]->send_signal));
		boardWorkerData[b]->send_signal = true;
}

/**
	This method turns the transducers off (so that the board does not heat up/die misserably)
	The board is still connected, so it can later be used again (e.g. create new traps)
*/
void AsierInhoImpl_V3::turnTransducersOff() {
	//0. Check status
	if (status != AsierInhoState::CONNECTED)
		return;

	//1. Set all phases and amplitudes to "0"== OFF and send!
	unsigned char* message = new unsigned char[512*numBoards];
	memset(message, 0, 512*numBoards* sizeof(unsigned char));	//Set all phases and amplitudes.
	for (int b = 0; b < numBoards; b++) {
		message[0 + 512 * b] = 0 + 128;	//Set flag indicating new update (>128)
		message[25 + 512 * b] = 128;	//Set flag indicating to change the LED colour (turn off in this case)

	}
	//2. Send it: 
	updateMessage(message);
	delete message;
}

void AsierInhoImpl_V3::turnTransducersOn() {
	//0. Check status
	if (status != AsierInhoState::CONNECTED)
		return;

	//1. Set all phases and amplitudes to "0"== OFF and send!
	unsigned char* message = new unsigned char[512*numBoards];
	memset(message, 64, 512*numBoards* sizeof(unsigned char));	//Set all phases and amplitudes.
	for (int b = 0; b < numBoards; b++) {
		message[0 + 512 * b] =	64 + 128;	//Set flag indicating new update (>128)
		message[25 + 512 * b] = 64 + 128;		//Set flag indicating to change the LED colour (turn off in this case)

	}
	//2. Send it: 
	updateMessage(message);
	delete message;	
}

/**
	This method disconnects the board, closing COM ports.
*/
void AsierInhoImpl_V3::disconnect() {
	if (status != AsierInhoState::CONNECTED)
		return;
	//0. Notify threads to end and wait for them
	for (int b = 0; b < numBoards; b++) {
		boardWorkerData[b]->running = false;
		//pthread_mutex_unlock(&(boardWorkerData[b]->send_signal));
		boardWorkerData[b]->send_signal = true;
	}
	for (int b = 0; b < numBoards; b++) 
		pthread_join(boardWorkerThreads[b], NULL);
	//1. Destroy threads and related resources (e.g. signals)
	for (int b = 0; b < numBoards; b++) {
		//pthread_mutex_destroy(&(boardWorkerData[b]->send_signal));
		pthread_mutex_destroy(&(boardWorkerData[b]->done_signal));
		FT_Close(boardWorkerData[b]->board);
		delete boardWorkerData[b]->boardSN;
		delete boardWorkerData[b];
	}
	boardWorkerData.clear();
	boardWorkerThreads.clear();
	status = AsierInhoState::INIT;
}


/**
	Sends a command to the board to load new phases and amplitudes for its 256 transducers.
*/
void AsierInhoImpl_V3::_sendUpdate(FT_HANDLE& board, unsigned char* stream, size_t numMessages) {

	FT_STATUS ftStatus; DWORD dataWritten;
	//4. Send!
	ftStatus = FT_Write(board, stream, (DWORD)numMessages*AsierInhoImpl_V3::messageSize, &dataWritten);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl_V3::_sendUpdate error %d\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
}

