#include "Device.h"

Device::Device(const char *name) : deviceName(name) {
}

ErrorCode Device::begin(void) {
  this->errorCode = initDevice();
  return this->errorCode;
}

const char *Device::getName(void) {
  return deviceName;
}

ErrorCode Device::getErrorCode(void) {
  return this->errorCode;
}

Device::~Device() {
}
