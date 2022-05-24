/**
	Simple C Wrapper for Board controller.
	The methods are a direct transcription to those used by the C++ interface
	For a description of what each of them does, please refer to AsierInho.h
*/
#ifndef _ASIERINHO_V3_CWRAPPER
#define _ASIERINHO_V3_CWRAPPER
#include <AsierInho_V3_Prerequisites.h>

extern "C" {
		_AsierInho_Export_V3 bool AsierInho_CWrapper_Initialize();
		_AsierInho_Export_V3 bool AsierInho_CWrapper_Release();
		_AsierInho_Export_V3 void AsierInho_CWrapper_RegisterPrintFuncs(void(*p_Message)(const char*), void(*p_Warning)(const char*), void(*p_Error)(const char*));
		_AsierInho_Export_V3 AsierInho_V3_Handler AsierInho_CWrapper_createHandler();
		_AsierInho_Export_V3 void AsierInho_CWrapper_destroyHandler(AsierInho_V3_Handler h);
		//_AsierInho_Export_V3 void AsierInho_CWrapper_readAdjustments(AsierInho_V3_Handler h, int* transducerIds, int* phaseAdjust);
		_AsierInho_Export_V3 void AsierInho_CWrapper_readParameters(AsierInho_V3_Handler h, float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels);
		_AsierInho_Export_V3 void AsierInho_CWrapper_updateMessage(AsierInho_V3_Handler h, unsigned char* messages);
		_AsierInho_Export_V3 void AsierInho_CWrapper_updateMessages(AsierInho_V3_Handler h, unsigned char* messages, int numMessages);
		_AsierInho_Export_V3 void AsierInho_CWrapper_turnTransducersOn(AsierInho_V3_Handler h);
		_AsierInho_Export_V3 void AsierInho_CWrapper_turnTransducersOff(AsierInho_V3_Handler h);
		_AsierInho_Export_V3 bool AsierInho_CWrapper_connectTopBottom(AsierInho_V3_Handler h,  int bottomBoardID, int topBoardID);
		_AsierInho_Export_V3 bool AsierInho_CWrapper_connect(AsierInho_V3_Handler h,  int numBoards, int* boardIDs, float* matBoardToWorld4x4);
		_AsierInho_Export_V3 void AsierInho_CWrapper_disconnect(AsierInho_V3_Handler h);
		
};
#endif