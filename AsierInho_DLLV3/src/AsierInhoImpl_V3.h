#ifndef _ASIERINHO_IMPLEMENTATION
#define _ASIERINHO_IMPLEMENTATION
#include <AsierInho_V3_Prerequisites.h>
#include <AsierInho_V3.h>
#include <math.h> 
#include <stdio.h>
#include <pthread.h>
#include <vector>
#define _USE_MATH_DEFINES
#include "ftd2xx.h"
#include <../Helper/TimeFunctions.h>

using namespace AsierInho_V3;

struct worker_threadData {
	FT_HANDLE board;				//Port to write to
	char* boardSN;
	bool running = true;
	bool send_signal;	//This mutex controls access to our signal. 
	pthread_mutex_t done_signal;	//We send this signal, once we are done updating.
	unsigned char* dataStream;	//We read the phases/amplitudes to send from this buffer.	
	static const int UPS = 8000;
	int curUpdate;
	float timestamps[UPS];
	struct timeval referenceTime;
};

class AsierInhoImpl_V3 : public AsierInhoBoard_V3 {
	//ATTRIBUTES:
public:
	static const size_t messageSize = 512;
protected:
	std::vector<int> boardIDs;// botBoardID, topBoardID;
	int numBoards;// = 2;
	int numDiscreteLevels;
	//Callibration parameters...
	int* transducerIds;// [256 * numBoards];
	int* phaseAdjust;// [256 * numBoards];
	float* transducerPositions;//[3*256 * numBoards]
	float* amplitudeAdjust;
	std::vector<char*> serialNumbers;// bottomSerialNumber, *topSerialNumber;
	enum AsierInhoState { INIT = 0, CONNECTED };
	int status;
	//shared resource pointing towards the mesages that the boards need to send to update the boards.
	unsigned char* dataStream;// [messageSize*numBoards];		//It can fit up to the message to send (phases+amplitudes)
	
	//Worker threads: They wait untill notified to update phases, and send an event back (XXBoardDone_signal) when the update is complete.
	std::vector<pthread_t> boardWorkerThreads;// topBoardThread;
	std::vector<worker_threadData*> boardWorkerData;

public:
	AsierInhoImpl_V3();
	~AsierInhoImpl_V3();
	virtual bool connect(int bottomBoardID, int topBoardID);
	virtual bool connect(int numBoards, int* boardIDs, float* matBoardToWorld4x4);
	virtual void readParameters(float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels);
	virtual void updateMessage(unsigned char* messages);
	virtual void updateMessages(unsigned char* message, int numMessages) ;
	virtual void turnTransducersOn();
	virtual void turnTransducersOff();
	virtual void disconnect();
	static void _sendUpdate(FT_HANDLE& board, unsigned char* stream, size_t numMessages);
#ifdef _TIME_PROFILING
	virtual void _profileTimes();
#endif

protected:
	
	void readBoardParameters(int boardId, float* matToWorld, float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels, worker_threadData* thread_data);
	bool initFTDevice(FT_HANDLE& fthandle, char* serialNumber, bool syncMode = false);

};

#endif