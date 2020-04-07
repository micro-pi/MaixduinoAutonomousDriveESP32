#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include "../Module.h"

class WifiModule : public Module {
private:
public:
  WifiModule(const char *moduleName);
  ErrorCode initModule(void);
  void mainFunction(void);
  virtual ~WifiModule();
};

#endif