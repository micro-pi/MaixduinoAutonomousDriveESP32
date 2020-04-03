#include "Module.h"

Module::Module(const char *name) : moduleName(name) {
}

const char *Module::getName(void) {
  return moduleName;
}

Module::~Module(void) {
}