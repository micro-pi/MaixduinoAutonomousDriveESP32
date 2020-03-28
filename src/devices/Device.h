#ifndef DEVICE_H
#define DEVICE_H

class Device {
private:
  const char *deviceName;

public:
  Device(const char *name);
  virtual void begin(void) = 0;
  virtual const char *getName(void);
  virtual ~Device(void);
};

#endif