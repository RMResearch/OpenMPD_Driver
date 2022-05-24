#include <AsierInho_V2.h>
#include <src/AsierInhoImpl_V2.h>

AsierInho_V2::AsierInhoBoard_V2* AsierInho_V2::createAsierInho() {
	AsierInhoImpl_V2* result = new AsierInhoImpl_V2();
	return result;
}

void (*_printMessage_AsierInho) (const char*) = NULL;
void (*_printError_AsierInho)(const char*) = NULL;
void (*_printWarning_AsierInho)(const char*) = NULL;

void AsierInho_V2::printMessage(const char* str) {
	if (_printMessage_AsierInho) _printMessage_AsierInho(str);
}
void AsierInho_V2::printError(const char* str) {
	if (_printError_AsierInho) _printError_AsierInho(str);
}
void AsierInho_V2::printWarning(const char* str) {
	if (_printWarning_AsierInho) _printWarning_AsierInho(str);
}

void AsierInho_V2::RegisterPrintFuncs(void(*p_Message)(const char*), void(*p_Warning)(const char*), void(*p_Error)(const char*)) {
	if (!_printMessage_AsierInho)_printMessage_AsierInho = p_Message;
	if (!_printError_AsierInho)_printError_AsierInho = p_Error;
	if (!_printWarning_AsierInho)_printWarning_AsierInho = p_Warning;
	printMessage("Got message functions!");
}