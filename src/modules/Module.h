#ifndef MODULE_H
#define MODULE_H

#include "../common/common.h"
#include "project_cfg.h"

class Module {
private:
  const char *moduleName;
  ErrorCode errorCode;

public:
  Module(const char *name);
  /**
   * @brief Initialisation function
   * @return E_OK if initialization was successful, otherwise returns E_NOK 
   */
  virtual ErrorCode init(void);
  virtual ErrorCode initModule(void) = 0;
  virtual void mainFunction(void) = 0;
  virtual const char *getName(void);
  virtual ErrorCode getErrorCode(void);
  virtual ~Module();
};

#endif
