#ifndef K210_H
#define K210_H

#include "../../common/common.h"
#include "../Device.h"

#include <driver/spi_slave.h>

class K210 : public Device {
private:
  spi_slave_transaction_t slaveTransaction;
  spi_host_device_t host;
  TickType_t ticksToWait;

public:
  K210(const char *deviceName);
  void begin(void);
  void setHost(const spi_host_device_t host);
  void setTicksToWait(const TickType_t ticksToWait);
  esp_err_t transferFullDuplex(K210ESP32Data &spi0Esp32TxBuffer, K210ESP32Data &spi0Esp32RxBuffer);
  virtual ~K210(void);
};

#endif