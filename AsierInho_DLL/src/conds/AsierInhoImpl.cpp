#include <src/SerialNumberList.h>
#include <src/AsierInhoImpl.h>
#include <src/ParseBoardConfig.h>
#include <sstream>
#include <fstream>

const float AsierInhoImpl::BoardHeight = 0.2388f;
static char consoleLineBuffer[512];

/**
	Method run by each of the wroker threads
*/
void* worker_BoardUpdater(void* threadData) {
	//we read ou configuration
	struct worker_threadData* data = (struct worker_threadData*)threadData;
	pthread_mutex_lock(&data->send_signal);
	while (data->running) {
		//Wait for our signal
		pthread_cond_wait(&data->do_send, &data->send_signal);
		pthread_mutex_unlock(&data->send_signal);
		//Send update: 		
		AsierInhoImpl::_sendUpdate(data->board, data->dataStream, 1);
		//Send notification signal
		pthread_mutex_unlock(&data->done_signal);
		pthread_mutex_lock(&data->send_signal);
	}
	pthread_mutex_unlock(&data->send_signal);
	AsierInho::printMessage("AsierInho Worker finished!\n");
	return 0;
}


AsierInhoImpl::AsierInhoImpl() :AsierInhoBoard(), status(INIT), mode(NORMAL) {
	//Initialise configuration parameters for each thread: 
	topData = new struct worker_threadData;
	bottomData = new struct worker_threadData;
	bottomData->dataStream = &(this->dataStream[0]);
	topData->dataStream = &(this->dataStream[512]);	
}

AsierInhoImpl::~AsierInhoImpl() {
	disconnect();
	delete topData;
	delete bottomData;
}

/**
	Connects to the board, using the board type and ID specified.
	ID corresponds to the index we labeled on each board.
	Returns true if connection was succesfull.
*/
bool AsierInhoImpl::connect(BoardType boardType, int bottomBoardID, int topBoardID) {
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
			bottomData->send_signal = topData->send_signal;
			pthread_cond_t cond;
			pthread_cond_init(&cond, NULL);
			topData->do_send = bottomData->do_send = cond;
			pthread_mutex_init(&(topData->done_signal), NULL);
			pthread_mutex_init(&(bottomData->done_signal), NULL);
			//Create threads with their initialization parameters: 
			pthread_attr_t attrs;
			pthread_attr_init(&attrs);
			pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
			pthread_create(&topBoardThread, &attrs 	, &worker_BoardUpdater, ((void*)topData));
			pthread_create(&bottomBoardThread, &attrs 	, &worker_BoardUpdater, ((void*)bottomData));
			Sleep(100);
			//Send calibration data automatically
			modeReset(AsierInhoMode::MATD);
			sendCalibData();
			//Set it to normal operational mode.
			modeReset(AsierInhoMode::NORMAL);
			AsierInho::printMessage("AsierInho connected succesfully\n");
			return true;
		}
		else {
			AsierInho::printWarning("AsierInho connection failed\n");
			return false;
		}
	}
	AsierInho::printWarning("AsierInho is already connected\n");
	return false;
}

/**
Sets the board configuration.
*/
/*void AsierInhoImpl::setBoardConfig(BoardType boardType, int bottomBoardID, int topBoardID) {
	if (status != AsierInhoState::INIT)
		return;

	for (int j = 0; j < 16; j++) {
		for (int i = 0; i < 16; i++) {
			if (boardType == BoardType::BensDesign) {
				transducerIds[i + 16 * j] = BensMapping[j + 16 * i];
				transducerIds[i + 16 * j + 256] = BensMapping[j + 16 * (15 - i)] + 256;
				phaseAdjust[i + 16 * j] = BensPhaseAdjust[bottomBoardID - 1][j + 16 * i];
				phaseAdjust[i + 16 * j + 256] = BensPhaseAdjust[topBoardID - 1][j + 16 * (15 - i)];
			}
			else if (boardType == BoardType::AsiersDesign) {
				transducerIds[i + 16 * j] = AsiersMapping[i + 16 * j];
				transducerIds[i + 16 * j + 256] = AsiersMapping[(15 - i) + 16 * j] + 256;
				phaseAdjust[i + 16 * j] = AsiersPhaseAdjust[bottomBoardID - 1][(15 - i) + 16 * (15 - j)];
				phaseAdjust[i + 16 * j + 256] = AsiersPhaseAdjust[topBoardID - 1][i + 16 * (15 - j)];
			}
		}
	}

	if (boardType == BoardType::BensDesign) {
		bottomSerialNumber = BensSerialNumbers[bottomBoardID - 1];
		topSerialNumber = BensSerialNumbers[topBoardID - 1];
		phaseAdjustBottom = BensPhaseAdjust[bottomBoardID - 1];
		phaseAdjustTop = BensPhaseAdjust[topBoardID - 1];
	}
	else if (boardType == BoardType::AsiersDesign) {
		bottomSerialNumber = AsiersSerialNumbers[bottomBoardID - 1];
		topSerialNumber = AsiersSerialNumbers[topBoardID - 1];
		phaseAdjustBottom = AsiersPhaseAdjust[bottomBoardID - 1];
		phaseAdjustTop = AsiersPhaseAdjust[topBoardID - 1];
	}
}*/

/*void AsierInhoImpl::readAdjustments(int* transducerIds, int* phaseAdjust) {
	memcpy(transducerIds, this->transducerIds, 512 * sizeof(int));
	memcpy(phaseAdjust, this->phaseAdjust, 512 * sizeof(int));
}*/

void AsierInhoImpl::readParameters(float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels) {
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
bool AsierInhoImpl::initFTDevice(FT_HANDLE& fthandle, char* serialNumber, bool syncMode) {

	FT_STATUS status;
	status = FT_OpenEx(serialNumber, FT_OPEN_BY_SERIAL_NUMBER, &fthandle);
	if (status != FT_OK) {
		if (status == FT_INVALID_HANDLE)	sprintf_s<512>(consoleLineBuffer, "Could not open USB port, status not ok (%d - invalid handle...)\n", status);
		if (status == FT_DEVICE_NOT_FOUND)	sprintf_s<512>(consoleLineBuffer, "Could not open USB port, open status not ok (%d - device not found...)\n", status);
		if (status == FT_DEVICE_NOT_OPENED) sprintf_s<512>(consoleLineBuffer, "Could not open USB port, open status not ok (%d - device not opened...)\n", status);
		AsierInho::printWarning(consoleLineBuffer);
		return false;
	}
	AsierInho::printMessage("USB ports open\n");
	status = FT_ResetDevice(fthandle);
	if (status != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "Could not reset USB ports. Reset status %d\n", status);
		AsierInho::printWarning(consoleLineBuffer);
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
	AsierInho::printMessage("USB ports setup\n");
	return true;
}

/**
	Linear range mapping from our continuous phases [0..2PI) to range [0..32)
*/
unsigned char AsierInhoImpl::_discretizePhase(float phase) {
	static const float TWO_PI = 2 * ((float)M_PI);
	//Transform phase to [0..2PI] range
	float mod_phase = fmodf(phase, TWO_PI);
	if (mod_phase < 0)
		mod_phase += TWO_PI;
	// ... and then to a [0..1) range
	float normalized_Phase = mod_phase / TWO_PI;
	//Map normal range [0..1) to our discrete range [0..32)
	//int discrete = (int)roundf(normalized_Phase * 64);
	int discrete = (int)(normalized_Phase * 128);
	//Done
	return (unsigned char)(discrete);
}


int computeDistance(unsigned char discretePhase_t1, unsigned char discretePhase_t2) {
	int diff = discretePhase_t1 - discretePhase_t2;
	if (diff <= 64 && diff > -64)
		return diff;
	else if (diff <= -64)
		return diff + 128;
	else
		return diff - 128;
}

unsigned char correctRange(int phase) {
	if (phase > 128)return phase - 128;
	if (phase < 0)return phase + 128;
	return (unsigned char)phase;
}

void AsierInhoImpl::discretizePhases(float* phases, unsigned char* discretePhases) {
	//if (phases == NULL || status != AsierInhoState::CONNECTED)
	//	return;

	//1. We traverse each transducer and re-map it to the appropriate trasnducer index...
	for (int index = 0; index < 512; index++) {
		//We compute its "PIN order" index
		int PIN_index = transducerIds[index];
		//We update the phase according to transducer-specific offset (Note the offsets must be multiplied by PI)
		float correctedPhase = phases[index] - (float)(phaseAdjust[index] * M_PI / 180.0f);
		//discretize phase
		discretePhases[PIN_index] = _discretizePhase(correctedPhase);

	}

}

// duty cycle and obtained amplitudes are not linear
void AsierInhoImpl::discretizeAmplitudes(float* amplitudes, unsigned char* discreteAmplitudes) {
	for (int i = 0; i < 512; i++) {
		unsigned char discreteAmplitude = _discretizeAmplitude(amplitudes[i]);
		int pinIndex = transducerIds[i];
		discreteAmplitudes[pinIndex] = discreteAmplitude;
	}
}
unsigned char AsierInhoImpl::_discretizeAmplitude(float amplitude) {
	float step = (float)((64 * 2.0f / M_PI) * asin(amplitude));
	return (unsigned char)step;
}

// changing the duty cycle shifts the phase, which needs to be corrected
void AsierInhoImpl::correctPhasesShift(unsigned char* discretePhases, unsigned char* discreteAmplitudes) {
	for (int i = 0; i < 512; i++) {
		unsigned char shift;
		shift = (64 - discreteAmplitudes[i]) / 2;

		if (discretePhases[i] + shift < 128)
			discretePhases[i] = discretePhases[i] + shift;
		else
			discretePhases[i] = discretePhases[i] + shift - 128;
	}
}

/**
	This method updates AsierInho with the discretized phases specified as an argument.
	Thus, the method will simply send these discretized phases to the board.
*/
void AsierInhoImpl::updateDiscretePhases(unsigned char* discretePhases) {
	//0. Check status
	if (status != AsierInhoState::CONNECTED)
		return;
	/*
	pthread_mutex_lock(&(topData->done_signal));
	pthread_mutex_lock(&(bottomData->done_signal));
	//Update the buffer:
	pthread_mutex_lock(&(bottomData->send_signal));
	memcpy(dataStream, message, messageSize*numBoards);
	pthread_cond_broadcast(&(bottomData->do_send));//Unlock all threads (simultaneously) to send it.
	pthread_mutex_unlock(&(bottomData->send_signal));
	*/
	//Multithreaded version: 
	//Wait for them to finish
	pthread_mutex_lock(&(topData->done_signal));
	pthread_mutex_lock(&(bottomData->done_signal));
	//Copy data to local
	memcpy(bottomData->dataStream, discretePhases, 256);
	bottomData->dataStream[0] += 128;
	memset(bottomData->dataStream + 256, 64, 256);//All amplitudes=1
	memcpy(topData->dataStream, discretePhases + 256, 256);
	memset(topData->dataStream + 256, 64, 256);//All amplitudes=1
	topData->dataStream[0] += 128;
	//Send signals to notify worker threads to update phases
	pthread_mutex_lock(&(bottomData->send_signal));
	pthread_cond_broadcast(&(bottomData->do_send));
	pthread_mutex_unlock(&(bottomData->send_signal));

}

void AsierInhoImpl::updateDiscretePhasesAndAmplitudes(unsigned char* discretePhases, unsigned char* discreteAmplitudes) {
	//0. Check status
	if (status != AsierInhoState::CONNECTED)
		return;

	//Multithreaded version: 
	//Wait for them to finish
	pthread_mutex_lock(&(topData->done_signal));
	pthread_mutex_lock(&(bottomData->done_signal));

	//Copy data to local
	memcpy(bottomData->dataStream, discretePhases, 256);
	bottomData->dataStream[0] += 128;
	memcpy(bottomData->dataStream + 256, discreteAmplitudes, 256);

	memcpy(topData->dataStream, discretePhases + 256, 256);
	topData->dataStream[0] += 128;
	memcpy(topData->dataStream + 256, discreteAmplitudes + 256, 256);
	
	//Send signals to notify worker threads to update phases
	pthread_mutex_lock(&(bottomData->send_signal));
	pthread_cond_broadcast(&(bottomData->do_send));
	pthread_mutex_unlock(&(bottomData->send_signal));
}

void AsierInhoImpl::updateMessage(unsigned char* message) {
	//0. Check status
	if (status != AsierInhoState::CONNECTED )
		return;
	//Multithreaded version: 
	//Wait for them to finish
	pthread_mutex_lock(&(topData->done_signal));
	pthread_mutex_lock(&(bottomData->done_signal));
	//Update the buffer:
	pthread_mutex_lock(&(bottomData->send_signal));
	memcpy(dataStream, message, messageSize*numBoards);
	pthread_cond_broadcast(&(bottomData->do_send));//Unlock all threads (simultaneously) to send it.
	pthread_mutex_unlock(&(bottomData->send_signal));
}

/**
	This method turns the transducers off (so that the board does not heat up/die misserably)
	The board is still connected, so it can later be used again (e.g. create new traps)
*/
void AsierInhoImpl::turnTransducersOff() {
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

void AsierInhoImpl::turnTransducersOn() {
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
void AsierInhoImpl::disconnect() {
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
void AsierInhoImpl::_sendUpdate(FT_HANDLE& board, unsigned char* stream, size_t numMessages) {

	FT_STATUS ftStatus; DWORD dataWritten;
	//4. Send!
	ftStatus = FT_Write(board, stream, (DWORD)numMessages*AsierInhoImpl::messageSize, &dataWritten);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::_sendUpdate error %d\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
}

void AsierInhoImpl::modeReset(int mode) {
	if (status != AsierInhoState::CONNECTED)
		return;

	// set the command to change the mode (Noraml GS PAT or MATD)
	static unsigned char message[512];
	this->mode = mode;
	unsigned char resetCommand = (mode == AsierInhoMode::NORMAL ? 0 : 255);
	memset(message, resetCommand, 512);

	// send the messeges
	DWORD data_written; FT_STATUS ftStatus;
	ftStatus = FT_Write(topData->board, &message, 512, &data_written);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::modeReset error %d -> Top board send error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
	ftStatus = FT_Write(bottomData->board, &message, 512, &data_written);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::modeReset error %d -> Bottom board send error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
	Sleep(100);
}

//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////	MATD METHODS						//////////////////////////////////



void AsierInhoImpl::sendCalibData() {
	if (status == AsierInhoState::INIT)
		return;

	unsigned char message_bottom[257];
	unsigned char message_top[257];
	// The messages need to have header (252) to tell the FPGAs that sending the calibration data
	message_bottom[0] = CalibCommand;
	message_top[0] = CalibCommand;

	// Every 256 transducer need different calibration
	for (int index = 0; index < 256; index++) {
		// 1. phase correction
		float calib_bottom = (float)(phaseAdjustBottom[index] * M_PI / 180.0f);
		float calib_top = (float)(phaseAdjustTop[index] * M_PI / 180.0f);
		// 2. discretize the corrected phases
		unsigned char discreteCalib_bottom = _discretizePhase(calib_bottom);
		unsigned char discreteCalib_top = _discretizePhase(calib_top);
		// 3. store in the buffer
		message_bottom[index + 1] = discreteCalib_bottom;
		message_top[index + 1] = discreteCalib_top;
	}
	//ct_bottom.sendString(message_bottom, 257);
	FT_STATUS ftStatus; DWORD dataWritten;
	//4. Send!
	ftStatus = FT_Write(bottomData->board, message_bottom, 257, &dataWritten);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::sendCalibData error %d -> Bottom board send error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
	//ct_top.sendString(message_top, 257);
	ftStatus = FT_Write(topData->board, message_top, 257, &dataWritten);
	if (ftStatus != FT_OK){
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::sendCalibData error %d -> Top board send error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
}

unsigned char AsierInhoImpl::_discretizeAmplitudeMATD(float amplitude) {
	//Map normal range [0..1) to our discrete range [0..maxAmplitude]
	int discreteAmplitude = (int)roundf(amplitude * 255);
	//Done
	return (unsigned char)(discreteAmplitude);
}

void AsierInhoImpl::_encodeFrame(float x, float y, float z, float amplitude, float phase, unsigned char red, unsigned char green, unsigned char blue, unsigned char *message) {
	// round the positoin values to make them at 0.25 mm resolution
	int int_x = (int)round(x * 32.0 * 1000.0);
	int_x = (int_x < 0 ? 8192 + int_x : int_x);		// x is range from -128.0 to 128.0mm
	int int_y = (int)round(y * 32.0 * 1000.0);
	int_y = (int_y < 0 ? 8192 + int_y : int_y);		// y is range from -128.0 to 128.0mm
	int int_z = (int)round(z * 32.0 * 1000.0);		// z is range from 0.0 to 256.0mm

	// encode each parameters of the frame
	unsigned char x_7_0 = (unsigned char)(int_x % 256);
	unsigned char x_12_8 = (unsigned char)(int_x / 256);
	unsigned char y_2_0 = (unsigned char)(int_y % 8);
	unsigned char y_10_3 = (unsigned char)((int)(int_y / 8) % 256);
	unsigned char y_12_11 = (unsigned char)(int_y / 2048);
	unsigned char z_5_0 = (unsigned char)(int_z % 64);
	unsigned char z_12_6 = (unsigned char)(int_z / 64);

	unsigned char descreteAmplitude = _discretizeAmplitudeMATD(amplitude);
	unsigned char descretePhase = _discretizePhase(phase);

	message[0] = x_7_0;					// from 0th to 7th bits of the x coordinate
	message[1] = x_12_8 + y_2_0 * 32;	// from 8th to 12th bits of the x and 0th to 2th bits of the y coordinates
	message[2] = y_10_3;				// from 3rd to 10th bits of the y
	message[3] = y_12_11 + z_5_0 * 4;	// from 11th and 12th bits of the y and 0th to 5th bits of the z coordinates
	message[4] = z_12_6;				// from 6th and 12th bits of the z coordinate
	message[5] = descreteAmplitude;
	message[6] = descretePhase;
	message[7] = red;
	message[8] = green;
	message[9] = blue;
}

void AsierInhoImpl::updateFrame(vec3 position, vec3 colour, float amplitude, float phase, bool twinFlag) {
	if (status == AsierInhoState::INIT)
		return;

	static unsigned char message_bottom[11], message_top[11];
	static unsigned char message_swap[1] = { SwapCommand };
	static float zCenter = BoardHeight / 2.0f;

	// number of frames is 1
	message_bottom[0] = 1;
	message_top[0] = 1;
	float signature = twinFlag ? (float)M_PI : 0.0f;	// signature for the top board is PI when create a twin trap, 0 when create a focus point
	_encodeFrame(-position.y, position.x, position.z, amplitude, phase, (unsigned char)colour.r, (unsigned char)colour.g, (unsigned char)colour.b, &message_bottom[1]);	// swap x and y to match the Optitrack coordinate
	_encodeFrame(-position.y, -position.x, BoardHeight - position.z, amplitude, phase + signature, (unsigned char)colour.r, (unsigned char)colour.g, (unsigned char)colour.b, &message_top[1]);	// top board is rotated around x-axis

	// send the messeges
	FT_STATUS ftStatus; DWORD dataWritten;
	ftStatus = FT_Write(bottomData->board, message_bottom, 11, &dataWritten);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::updateFrame error %d -> Bottom board send error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
	ftStatus = FT_Write(topData->board, message_top, 11, &dataWritten);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::updateFrame error %d -> Top board send error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
	//Send byte to trigger swapping to new position.
	ftStatus = FT_Write(bottomData->board, message_swap, 1, &dataWritten);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::updateFrame error %d -> Bottom board send swap byte error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
	ftStatus = FT_Write(topData->board, message_swap, 1, &dataWritten);
	if (ftStatus != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::updateFrame error %d -> Top board send swap byte error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
}

void AsierInhoImpl::updateMultipleFrames(int numFramesPerUpdate, vec3 *positions, vec3 *colours, float *amplitudes, float *phases, bool *twinFlags) {
	if (status == AsierInhoState::INIT)
		return;

	static unsigned char message_bottom[2501], message_top[2501];
	static unsigned char message_swap[1] = { SwapCommand };
	static float zCenter = BoardHeight / 2.0f;

	// number of frames needs to be sent first
	message_bottom[0] = (unsigned char)numFramesPerUpdate;
	message_top[0] = (unsigned char)numFramesPerUpdate;
	for (int i = 0; i < numFramesPerUpdate; i++) {
		vec3 colour = colours == NULL ? vec3(0, 0, 0) : colours[i];
		float amplitude = amplitudes == NULL ? 1.f : amplitudes[i];
		float phase = phases == NULL ? 0.f : phases[i];
		float signature = (twinFlags == NULL || twinFlags[i]) ? (float)M_PI : 0.0f;	// signature for the top board is PI when create a twin trap, 0 when create a focus point
		_encodeFrame(-positions[i].y, positions[i].x, positions[i].z, amplitude, phase, (unsigned char)colour.r, (unsigned char)colour.g, (unsigned char)colour.b, &message_bottom[1 + i * 10]);	// swap x and y to match the Optitrack coordinate
		_encodeFrame(-positions[i].y, -positions[i].x, BoardHeight - positions[i].z, amplitude, phase + signature, (unsigned char)colour.r, (unsigned char)colour.g, (unsigned char)colour.b, &message_top[1 + i * 10]);	// top board is rotated around x-axis
	}

	// send the messeges
	FT_STATUS ftStatus; DWORD dataWritten;
	if ((ftStatus = FT_Write(bottomData->board, message_bottom, 10 * numFramesPerUpdate + 1, &dataWritten)) != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::updateMultipleFrames error %d -> Bottom board send update error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}		
	if ((ftStatus = FT_Write(topData->board, message_top, 10 * numFramesPerUpdate + 1, &dataWritten)) != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::updateMultipleFrames error %d -> Top board send update  error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
	if ((ftStatus = FT_Write(bottomData->board, message_swap, 1, &dataWritten)) != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::updateMultipleFrames error %d -> Bottom board send swap byte error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}
	if ((ftStatus = FT_Write(topData->board, message_swap, 1, &dataWritten)) != FT_OK) {
		sprintf_s<512>(consoleLineBuffer, "AsierInhoImpl::updateMultipleFrames error %d -> Top board send swap byte error\n", ftStatus);
		printWarning(consoleLineBuffer);
	}

}
