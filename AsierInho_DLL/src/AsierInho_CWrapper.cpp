#include <AsierInho_Prerequisites.h>
#include <AsierInho_CWrapper.h>
#include <AsierInho.h>
#include <stdio.h>
#include <list> 
using namespace AsierInho;

static std::list<AsierInhoBoard*> handlers;

bool AsierInho_CWrapper_Initialize() {
	return true;
}

bool AsierInho_CWrapper_Release() {
	//1. Delete all handlers in our lists
	std::list<AsierInhoBoard*>::iterator itHandlers;
	for (itHandlers = handlers.begin(); itHandlers != handlers.end(); itHandlers++)
		delete(*itHandlers);
	//2.Reset the list to empty
	handlers.clear();	
	return true;
}

void AsierInho_CWrapper_RegisterPrintFuncs(void(*p_Message)(const char*), void(*p_Warning)(const char*), void(*p_Error)(const char*)) {
	AsierInho::RegisterPrintFuncs(p_Message, p_Warning, p_Error);
}


AsierInho_Handler AsierInho_CWrapper_createHandler() {
	//Create a new handler and add to list. 
	AsierInhoBoard* result = AsierInho::createAsierInho();
	handlers.push_back(result);
	return (AsierInho_Handler)(result);
}

void AsierInho_CWrapper_destroyHandler(AsierInho_Handler h) {
	AsierInhoBoard* target = (AsierInho::AsierInhoBoard*)h;
	handlers.remove(target);
	delete target;		
}

/*void AsierInho_CWrapper_readAdjustments(AsierInho_Handler h, int* transducerIds, int* phaseAdjust) {
	((AsierInho::AsierInhoBoard*)h)->readAdjustments(transducerIds, phaseAdjust);
}*/

void AsierInho_CWrapper_readParameters(AsierInho_Handler h, float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels){
	((AsierInho::AsierInhoBoard*)h)->readParameters(transducerPositions, transducerIds, phaseAdjust, amplitudeAdjust, numDiscreteLevels);
}

void AsierInho_CWrapper_modeReset(AsierInho_Handler h, int mode) {
	((AsierInho::AsierInhoBoard*)h)->modeReset(mode);
}

void AsierInho_CWrapper_updateMessage(AsierInho_Handler h, unsigned char* messages) {
	((AsierInho::AsierInhoBoard*)h)->updateMessage(messages);
}

void AsierInho_CWrapper_turnTransducersOn(AsierInho_Handler h) {
	((AsierInho::AsierInhoBoard*)h)->turnTransducersOn();
}

void AsierInho_CWrapper_turnTransducersOff(AsierInho_Handler h) {
	((AsierInho::AsierInhoBoard*)h)->turnTransducersOff();
}

bool AsierInho_CWrapper_connect(AsierInho_Handler h, AsierInho::BoardType boardType, int bottomBoardID, int topBoardID) {
	return (((AsierInho::AsierInhoBoard*)h)->connect(boardType, bottomBoardID, topBoardID));
}

void AsierInho_CWrapper_disconnect(AsierInho_Handler h) {
	((AsierInho::AsierInhoBoard*)h)->disconnect();
}

float AsierInho_CWrapper_boardHeight(AsierInho_Handler h) {
	return ((AsierInho::AsierInhoBoard*)h)->boardHeight();
}

void AsierInho_CWrapper_updateFrame(AsierInho_Handler h, vec3 position, vec3 colour, float amplitude, float phase, bool twinTrap) {
	((AsierInho::AsierInhoBoard*)h)->updateFrame(position, colour, amplitude, phase, twinTrap);
}

void AsierInho_CWrapper_updateMultipleFrames(AsierInho_Handler h, int numFramesPerUpdate, vec3 *positions, vec3 *colours, float *amplitudes, float *phases, bool *twinFlags) {
	((AsierInho::AsierInhoBoard*)h)->updateMultipleFrames(numFramesPerUpdate, positions, colours, amplitudes, phases, twinFlags);
}

void AsierInho_CWrapper_discretizePhases(AsierInho_Handler h, float* phases, unsigned char* discretePhases) {
	((AsierInho::AsierInhoBoard*)h)->discretizePhases(phases, discretePhases);
}

unsigned char AsierInho_CWrapper_discretizePhase(AsierInho_Handler h, float phase) {
	return ((AsierInho::AsierInhoBoard*)h)->_discretizePhase(phase);
}

void AsierInho_CWrapper_discretizeAmplitudes(AsierInho_Handler h, float* amplitudes, unsigned char* discreteAmplitudes) {
	((AsierInho::AsierInhoBoard*)h)->discretizeAmplitudes(amplitudes, discreteAmplitudes);
}

unsigned char AsierInho_CWrapper_discretizeAmplitude(AsierInho_Handler h, float amplitude) {
	return ((AsierInho::AsierInhoBoard*)h)->_discretizeAmplitude(amplitude);
}

void AsierInho_CWrapper_correctPhasesShift(AsierInho_Handler h, unsigned char* discretePhases, unsigned char* discreteAmplitudes) {
	((AsierInho::AsierInhoBoard*)h)->correctPhasesShift(discretePhases, discreteAmplitudes);
}

void AsierInho_CWrapper_updateDiscretePhases(AsierInho_Handler h, unsigned char* discretePhases) {
	((AsierInho::AsierInhoBoard*)h)->updateDiscretePhases(discretePhases);
}

void AsierInho_CWrapper_updateDiscretePhasesAndAmplitudes(AsierInho_Handler h, unsigned char* discretePhases, unsigned char* discreteAmplitudes) {
	((AsierInho::AsierInhoBoard*)h)->updateDiscretePhasesAndAmplitudes(discretePhases, discreteAmplitudes);
}
