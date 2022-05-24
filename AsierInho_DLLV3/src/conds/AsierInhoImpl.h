#ifndef _ASIERINHO_IMPLEMENTATION
#define _ASIERINHO_IMPLEMENTATION
#include <AsierInho_Prerequisites.h>
#include <AsierInho.h>
#include <math.h> 
#include <stdio.h>
#include <pthread.h>
#define _USE_MATH_DEFINES
#include "ftd2xx.h"

using namespace AsierInho;

struct worker_threadData {
	FT_HANDLE board;				//Port to write to
	bool running = true;
	pthread_mutex_t send_signal;	//This mutex controls access to our signal. 
	pthread_cond_t do_send;			//We wait on this signal, to know when to update the phases in our board.
	pthread_mutex_t done_signal;	//We send this signal, once we are done updating.
	unsigned char* dataStream;	//We read the phases/amplitudes to send from this buffer.
	
};

class AsierInhoImpl : public AsierInhoBoard {
	//ATTRIBUTES:
public:
	static const int CalibCommand = 252;			// Commands for sending the calibration data
	static const int SwapCommand = 251;				// Commands for sending the swap signal
	static const float BoardHeight;
	static const size_t messageSize = 512;
protected:
	int botBoardID, topBoardID;
	static const int numBoards = 2;
	//Callibration parameters...
	int transducerIds[256*numBoards];
	int phaseAdjust[256*numBoards];
	char *bottomSerialNumber, *topSerialNumber;
	int* phaseAdjustBottom, * phaseAdjustTop;
	enum AsierInhoState { INIT = 0, CONNECTED };
	int status;
	int mode;
	//shared resource pointing towards the mesages that the boards need to send to update the boards.
	unsigned char dataStream[messageSize*numBoards];		//It can fit up to the message to send (phases+amplitudes)
	
	//Worker threads: They wait untill notified to update phases, and send an event back (XXBoardDone_signal) when the update is complete.
	pthread_t topBoardThread;
	pthread_t bottomBoardThread;
	worker_threadData* topData;
	worker_threadData* bottomData;
	//void setBoardConfig(BoardType boardType, int bottomBoardID, int topBoardID);
	/**
		This method sends the calibration data (tansducer mapping and pahse delays) for each transducer board.
		This needs to be called after changing the board mode to the MATD mode.
	*/
	void sendCalibData();

	//METHODS:
public:
	AsierInhoImpl();
	~AsierInhoImpl();
	virtual bool connect(BoardType boardType, int bottomBoardID, int topBoardID);
	virtual void readParameters(float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels);
	//virtual void readAdjustments(int* transducerIds, int* phaseAdjust);
	bool initFTDevice(FT_HANDLE& fthandle, char* serialNumber, bool syncMode = false);
	virtual void discretizePhases(float* phases, unsigned char* discretePhases);
	virtual unsigned char _discretizePhase(float phase);
	virtual void discretizeAmplitudes(float* amplitudes, unsigned char* discreteAmplitudes);
	virtual unsigned char _discretizeAmplitude(float amplitude);
	virtual void correctPhasesShift(unsigned char* discretePhases, unsigned char* discreteAmplitudes);
	virtual void updateDiscretePhases(unsigned char* discretePhases);
	virtual void updateDiscretePhasesAndAmplitudes(unsigned char* discretePhases, unsigned char* discreteAmplitudes);
	virtual void updateMessage(unsigned char* messages);
	virtual void turnTransducersOn();
	virtual void turnTransducersOff();
	virtual void disconnect();
	static void _sendUpdate(FT_HANDLE& board, unsigned char* stream, size_t numMessages);
	virtual void modeReset(int mode);
	unsigned char _discretizeAmplitudeMATD(float amplitude);
	void _encodeFrame(float x, float y, float z, float amplitude, float phase, unsigned char red, unsigned char green, unsigned char blue, unsigned char *message);
	virtual float boardHeight() { return this->BoardHeight; }
	virtual void updateFrame(vec3 position, vec3 colour = vec3(0, 0, 0), float amplitude = 1.0f, float phase = 0.0f, bool twinTrap = true);
	virtual void updateMultipleFrames(int numFramesPerUpdate, vec3 *positions = NULL, vec3 *colours = NULL, float *amplitudes = NULL, float *phases = NULL, bool *twinFlags = nullptr);
};

#endif