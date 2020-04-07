#include "Module.h"

Module::Module(const char *name) : moduleName(name) {
  this->errorCode = E_NOK;
}

ErrorCode Module::init(void) {
  this->errorCode = initModule();
  return this->errorCode;
}

const char *Module::getName(void) {
  return this->moduleName;
}

ErrorCode Module::getErrorCode(void) {
  return this->errorCode;
}

Module::~Module(void) {
}