#ifndef K210_ESP32_COMMUNICATION_H
#define K210_ESP32_COMMUNICATION_H

#include "../../devices/K210/K210.h"
#include "../Module.h"
#include <freertos/FreeRTOS.h>

class K210ESP32Communication : public Module {
private:
  xQueueHandle movingModuleCommandsQueue;
  MovingModuleInterface movingModuleInterface;
  K210ESP32Data spi0Esp32TxBuffer;
  K210ESP32Data spi0Esp32RxBuffer;
  K210 *k210;

public:
  /**
   * @brief Default constructor
   */
  K210ESP32Communication(const char *moduleName);
  ErrorCode init(void);
  void setMovingModuleCommandsQueue(xQueueHandle movingModuleCommandsQueue);
  void setK210Device(K210 &k210);
  void mainFunction(void);
  /**
   * @brief Destructor
   */
  virtual ~K210ESP32Communication(void);
};

#endif