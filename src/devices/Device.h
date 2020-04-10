#ifndef DEVICE_H
#define DEVICE_H

#include "../common/common.h"

class Device {
private:
  const char *deviceName;
  ErrorCode errorCode;

public:
  Device(const char *name);
  virtual ErrorCode begin(void);
  virtual ErrorCode initDevice(void) = 0;
  virtual const char *getName(void);
  virtual ErrorCode getErrorCode(void);
  virtual ~Device();
};

#endif
