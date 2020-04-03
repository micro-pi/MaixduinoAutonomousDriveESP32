#include "WifiModule.h"

WifiModule::WifiModule(const char *moduleName) : Module(moduleName) {
}

ErrorCode WifiModule::init(void) {
  return E_OK;
}

void WifiModule::mainFunction(void) {
}

WifiModule::~WifiModule() {
}