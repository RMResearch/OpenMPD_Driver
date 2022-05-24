#include <AsierInho_V3_Prerequisites.h>
#include <AsierInho_V3_CWrapper.h>
#include <AsierInho_V3.h>
#include <stdio.h>
#include <list> 
using namespace AsierInho_V3;

static std::list<AsierInhoBoard_V3*> handlers;

bool AsierInho_CWrapper_Initialize() {
	return true;
}

bool AsierInho_CWrapper_Release() {
	//1. Delete all handlers in our lists
	std::list<AsierInhoBoard_V3*>::iterator itHandlers;
	for (itHandlers = handlers.begin(); itHandlers != handlers.end(); itHandlers++)
		delete(*itHandlers);
	//2.Reset the list to empty
	handlers.clear();	
	return true;
}

void AsierInho_CWrapper_RegisterPrintFuncs(void(*p_Message)(const char*), void(*p_Warning)(const char*), void(*p_Error)(const char*)) {
	AsierInho_V3::RegisterPrintFuncs(p_Message, p_Warning, p_Error);
}


AsierInho_V3_Handler AsierInho_CWrapper_createHandler() {
	//Create a new handler and add to list. 
	AsierInhoBoard_V3* result = AsierInho_V3::createAsierInho();
	handlers.push_back(result);
	return (AsierInho_V3_Handler)(result);
}

void AsierInho_CWrapper_destroyHandler(AsierInho_V3_Handler h) {
	AsierInhoBoard_V3* target = (AsierInho_V3::AsierInhoBoard_V3*)h;
	handlers.remove(target);
	delete target;		
}


void AsierInho_CWrapper_readParameters(AsierInho_V3_Handler h, float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels){
	((AsierInho_V3::AsierInhoBoard_V3*)h)->readParameters(transducerPositions, transducerIds, phaseAdjust, amplitudeAdjust, numDiscreteLevels);
}

void AsierInho_CWrapper_updateMessage(AsierInho_V3_Handler h, unsigned char* messages) {
	((AsierInho_V3::AsierInhoBoard_V3*)h)->updateMessage(messages);
}

void AsierInho_CWrapper_updateMessages(AsierInho_V3_Handler h, unsigned char* messages, int numMessages) {
	((AsierInho_V3::AsierInhoBoard_V3*)h)->updateMessages(messages, numMessages);
}

void AsierInho_CWrapper_turnTransducersOn(AsierInho_V3_Handler h) {
	((AsierInho_V3::AsierInhoBoard_V3*)h)->turnTransducersOn();
}

void AsierInho_CWrapper_turnTransducersOff(AsierInho_V3_Handler h) {
	((AsierInho_V3::AsierInhoBoard_V3*)h)->turnTransducersOff();
}

bool AsierInho_CWrapper_connectTopBottom(AsierInho_V3_Handler h, int bottomBoardID, int topBoardID) {
	return (((AsierInho_V3::AsierInhoBoard_V3*)h)->connect( bottomBoardID, topBoardID));
}

bool AsierInho_CWrapper_connect(AsierInho_V3_Handler h, int numBoards, int* boardIDs, float* matBoardToWorld4x4) {
	return (((AsierInho_V3::AsierInhoBoard_V3*)h)->connect(numBoards, boardIDs, matBoardToWorld4x4));
}

void AsierInho_CWrapper_disconnect(AsierInho_V3_Handler h) {
	((AsierInho_V3::AsierInhoBoard_V3*)h)->disconnect();
}


