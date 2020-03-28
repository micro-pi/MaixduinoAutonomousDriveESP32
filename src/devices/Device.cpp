#include "Device.h"

Device::Device(const char *name) : deviceName(name) {
}

const char *Device::getName(void) {
  return deviceName;
}

Device::~Device(void) {
}