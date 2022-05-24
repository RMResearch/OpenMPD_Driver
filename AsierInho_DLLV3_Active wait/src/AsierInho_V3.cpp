#include <AsierInho_V3.h>
#include <src/AsierInhoImpl_V3.h>

AsierInho_V3::AsierInhoBoard_V3* AsierInho_V3::createAsierInho() {
	AsierInhoImpl_V3* result = new AsierInhoImpl_V3();
	return result;
}

void (*_printMessage_AsierInho) (const char*) = NULL;
void (*_printError_AsierInho)(const char*) = NULL;
void (*_printWarning_AsierInho)(const char*) = NULL;

void AsierInho_V3::printMessage(const char* str) {
	if (_printMessage_AsierInho) _printMessage_AsierInho(str);
}
void AsierInho_V3::printError(const char* str) {
	if (_printError_AsierInho) _printError_AsierInho(str);
}
void AsierInho_V3::printWarning(const char* str) {
	if (_printWarning_AsierInho) _printWarning_AsierInho(str);
}

void AsierInho_V3::RegisterPrintFuncs(void(*p_Message)(const char*), void(*p_Warning)(const char*), void(*p_Error)(const char*)) {
	if (!_printMessage_AsierInho)_printMessage_AsierInho = p_Message;
	if (!_printError_AsierInho)_printError_AsierInho = p_Error;
	if (!_printWarning_AsierInho)_printWarning_AsierInho = p_Warning;
	printMessage("Got message functions!");
}