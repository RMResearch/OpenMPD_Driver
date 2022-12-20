#include <AsierInho_V2_Prerequisites.h>
#include <AsierInho_V2_CWrapper.h>
#include <AsierInho_V2.h>
#include <stdio.h>
#include <list> 
using namespace AsierInho_V2;

static std::list<AsierInhoBoard_V2*> handlers;

bool AsierInho_CWrapper_Initialize() {
	return true;
}

bool AsierInho_CWrapper_Release() {
	//1. Delete all handlers in our lists
	std::list<AsierInhoBoard_V2*>::iterator itHandlers;
	for (itHandlers = handlers.begin(); itHandlers != handlers.end(); itHandlers++)
		delete(*itHandlers);
	//2.Reset the list to empty
	handlers.clear();	
	return true;
}

void AsierInho_CWrapper_RegisterPrintFuncs(void(*p_Message)(const char*), void(*p_Warning)(const char*), void(*p_Error)(const char*)) {
	AsierInho_V2::RegisterPrintFuncs(p_Message, p_Warning, p_Error);
}


AsierInho_V2_Handler AsierInho_CWrapper_createHandler() {
	//Create a new handler and add to list. 
	AsierInhoBoard_V2* result = AsierInho_V2::createAsierInho();
	handlers.push_back(result);
	return (AsierInho_V2_Handler)(result);
}

void AsierInho_CWrapper_destroyHandler(AsierInho_V2_Handler h) {
	AsierInhoBoard_V2* target = (AsierInho_V2::AsierInhoBoard_V2*)h;
	handlers.remove(target);
	delete target;		
}


void AsierInho_CWrapper_readParameters(AsierInho_V2_Handler h, float* transducerPositions, float* transducerNormals, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels){
	((AsierInho_V2::AsierInhoBoard_V2*)h)->readParameters(transducerPositions, transducerNormals, transducerIds, phaseAdjust, amplitudeAdjust, numDiscreteLevels);
}

_AsierInho_Export_V2 size_t AsierInho_CWrapper_totalTransducers(AsierInho_V2_Handler h)
{
	return ((AsierInho_V2::AsierInhoBoard_V2*)h)->totalTransducers();
}

_AsierInho_Export_V2 size_t AsierInho_CWrapper_totalBoards(AsierInho_V2_Handler h)
{
	return ((AsierInho_V2::AsierInhoBoard_V2*)h)->totalBoards();
}

_AsierInho_Export_V2 void AsierInho_CWrapper_numTransducersPerBoard(AsierInho_V2_Handler h, size_t * out_TransPerBoard)
{
	((AsierInho_V2::AsierInhoBoard_V2*)h)->numTransducersPerBoard(out_TransPerBoard);
}

void AsierInho_CWrapper_updateMessage(AsierInho_V2_Handler h, unsigned char* messages) {
	((AsierInho_V2::AsierInhoBoard_V2*)h)->updateMessage(messages);
}

void AsierInho_CWrapper_updateMessages(AsierInho_V2_Handler h, unsigned char* messages, int numMessages) {
	((AsierInho_V2::AsierInhoBoard_V2*)h)->updateMessages(messages, numMessages);
}

void AsierInho_CWrapper_turnTransducersOn(AsierInho_V2_Handler h) {
	((AsierInho_V2::AsierInhoBoard_V2*)h)->turnTransducersOn();
}

void AsierInho_CWrapper_turnTransducersOff(AsierInho_V2_Handler h) {
	((AsierInho_V2::AsierInhoBoard_V2*)h)->turnTransducersOff();
}

bool AsierInho_CWrapper_connectTopBottom(AsierInho_V2_Handler h, int bottomBoardID, int topBoardID) {
	return (((AsierInho_V2::AsierInhoBoard_V2*)h)->connect( bottomBoardID, topBoardID));
}

bool AsierInho_CWrapper_connect(AsierInho_V2_Handler h, int numBoards, int* boardIDs, float* matBoardToWorld4x4) {
	return (((AsierInho_V2::AsierInhoBoard_V2*)h)->connect(numBoards, boardIDs, matBoardToWorld4x4));
}

void AsierInho_CWrapper_disconnect(AsierInho_V2_Handler h) {
	((AsierInho_V2::AsierInhoBoard_V2*)h)->disconnect();
}


