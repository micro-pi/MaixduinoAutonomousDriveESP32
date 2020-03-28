#include "K210.h"

K210::K210(const char *deviceName) : Device(deviceName) {
  this->slaveTransaction.length = 0;
  this->slaveTransaction.trans_len = 0;
  this->slaveTransaction.tx_buffer = nullptr;
  this->slaveTransaction.rx_buffer = nullptr;
  this->slaveTransaction.user = nullptr;
  this->host = VSPI_HOST;
  this->ticksToWait = portMAX_DELAY;
}

void K210::begin() {
}

void K210::setHost(const spi_host_device_t host) {
  this->host = host;
}

void K210::setTicksToWait(const TickType_t ticksToWait) {
  this->ticksToWait = ticksToWait;
}

esp_err_t K210::transferFullDuplex(K210ESP32Data &spi0Esp32TxBuffer, K210ESP32Data &spi0Esp32RxBuffer) {
  spi0Esp32TxBuffer.crc = k210Esp32DataCrc16(spi0Esp32TxBuffer);

  slaveTransaction.length = SPI_BUF_SIZE * 8;
  slaveTransaction.tx_buffer = &spi0Esp32TxBuffer;
  slaveTransaction.rx_buffer = &spi0Esp32RxBuffer;

  return spi_slave_transmit(host, &slaveTransaction, ticksToWait);
}

K210::~K210() {
}