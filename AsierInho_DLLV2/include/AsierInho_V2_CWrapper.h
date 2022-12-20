/**
	Simple C Wrapper for Board controller.
	The methods are a direct transcription to those used by the C++ interface
	For a description of what each of them does, please refer to AsierInho.h
*/
#ifndef _ASIERINHO_V2_CWRAPPER
#define _ASIERINHO_V2_CWRAPPER
#include <AsierInho_V2_Prerequisites.h>

extern "C" {
		_AsierInho_Export_V2 bool AsierInho_CWrapper_Initialize();
		_AsierInho_Export_V2 bool AsierInho_CWrapper_Release();
		_AsierInho_Export_V2 void AsierInho_CWrapper_RegisterPrintFuncs(void(*p_Message)(const char*), void(*p_Warning)(const char*), void(*p_Error)(const char*));
		_AsierInho_Export_V2 AsierInho_V2_Handler AsierInho_CWrapper_createHandler();
		_AsierInho_Export_V2 void AsierInho_CWrapper_destroyHandler(AsierInho_V2_Handler h);
		//_AsierInho_Export_V2 void AsierInho_CWrapper_readAdjustments(AsierInho_V2_Handler h, int* transducerIds, int* phaseAdjust);
		_AsierInho_Export_V2 void AsierInho_CWrapper_readParameters(AsierInho_V2_Handler h, float* transducerPositions, float* transducerNormals, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels);
		_AsierInho_Export_V2 size_t AsierInho_CWrapper_totalTransducers(AsierInho_V2_Handler h);
		_AsierInho_Export_V2 size_t AsierInho_CWrapper_totalBoards(AsierInho_V2_Handler h);
		_AsierInho_Export_V2 void AsierInho_CWrapper_numTransducersPerBoard(AsierInho_V2_Handler h, size_t* out_TransPerBoard);
		_AsierInho_Export_V2 void AsierInho_CWrapper_updateMessage(AsierInho_V2_Handler h, unsigned char* message);
		_AsierInho_Export_V2 void AsierInho_CWrapper_updateMessages(AsierInho_V2_Handler h, unsigned char* messages, int numMessages);
		_AsierInho_Export_V2 void AsierInho_CWrapper_turnTransducersOn(AsierInho_V2_Handler h);
		_AsierInho_Export_V2 void AsierInho_CWrapper_turnTransducersOff(AsierInho_V2_Handler h);
		_AsierInho_Export_V2 bool AsierInho_CWrapper_connectTopBottom(AsierInho_V2_Handler h,  int bottomBoardID, int topBoardID);
		_AsierInho_Export_V2 bool AsierInho_CWrapper_connect(AsierInho_V2_Handler h,  int numBoards, int* boardIDs, float* matBoardToWorld4x4);
		_AsierInho_Export_V2 void AsierInho_CWrapper_disconnect(AsierInho_V2_Handler h);
		
};
#endif