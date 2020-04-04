#include "K210ESP32Communication.h"

#include <esp_log.h>
#include <string.h>

static const char *TAG = "KECM_ESP32";

K210ESP32Communication::K210ESP32Communication(const char *moduleName) : Module(moduleName) {
  this->movingModuleCommandsQueue = nullptr;
  this->k210 = nullptr;
}

ErrorCode K210ESP32Communication::init(void) {
  ErrorCode errorCode;
  errorCode = E_OK;

  return errorCode;
}

void K210ESP32Communication::setMovingModuleCommandsQueue(xQueueHandle movingModuleCommandsQueue) {
  this->movingModuleCommandsQueue = movingModuleCommandsQueue;
}

void K210ESP32Communication::setK210Device(K210 &k210) {
  this->k210 = &k210;
}

void K210ESP32Communication::mainFunction(void) {
  esp_err_t espErr;
  uint8_t ctx = 0;

  if (k210 != nullptr) {
    /* TODO: Create xQueue of tx messages (CMD's, Messages etc.) */
    // if (ctx == 2) {
    //   movingModuleInterface.command = MOVING_MODULE_COMMAND_MOVE;
    //   movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
    //   movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_FORWARD;
    //   movingModuleInterface.pwmValue = (uint16_t)(0.4 * 1000);

    //   memcpy(spi0Esp32TxBuffer.data, &movingModuleInterface, sizeof(MovingModuleInterface));
    //   spi0Esp32TxBuffer.type = MOVING_CMD;
    //   spi0Esp32TxBuffer.id = (uint8_t)xLastWakeTime;
    //   spi0Esp32TxBuffer.size = sizeof(MovingModuleInterface);
    // } else if (ctx == 5) {
    //   movingModuleInterface.command = MOVING_MODULE_COMMAND_PWM;
    //   movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
    //   movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_NONE;
    //   movingModuleInterface.pwmValue = (uint16_t)(0.35 * 1000);

    //   memcpy(spi0Esp32TxBuffer.data, &movingModuleInterface, sizeof(MovingModuleInterface));
    //   spi0Esp32TxBuffer.type = MOVING_CMD;
    //   spi0Esp32TxBuffer.id = (uint8_t)xLastWakeTime;
    //   spi0Esp32TxBuffer.size = sizeof(MovingModuleInterface);
    // } else if (ctx == 10) {
    //   movingModuleInterface.command = MOVING_MODULE_COMMAND_STOP;
    //   movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
    //   movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_NONE;
    //   movingModuleInterface.pwmValue = 0;

    //   memcpy(spi0Esp32TxBuffer.data, &movingModuleInterface, sizeof(MovingModuleInterface));
    //   spi0Esp32TxBuffer.type = MOVING_CMD;
    //   spi0Esp32TxBuffer.id = (uint8_t)xLastWakeTime;
    //   spi0Esp32TxBuffer.size = sizeof(MovingModuleInterface);
    // } else {
    sprintf((char *)spi0Esp32TxBuffer.data, "Hello K210, xLastWakeTime");
    spi0Esp32TxBuffer.type = STRING;
    spi0Esp32TxBuffer.id = (uint8_t)1;
    spi0Esp32TxBuffer.size = strlen((char *)spi0Esp32TxBuffer.data);
    // }

    espErr = k210->transferFullDuplex(spi0Esp32TxBuffer, spi0Esp32RxBuffer);
    if (espErr == ESP_OK) {
      ESP_LOGI(TAG, "Rx.id  : %d ", spi0Esp32RxBuffer.id);
      ESP_LOGI(TAG, "Rx.type: %d ", spi0Esp32RxBuffer.type);
      ESP_LOGI(TAG, "Rx.size: %d ", spi0Esp32RxBuffer.size);
      ESP_LOGI(TAG, "Rx.data: %s ", (char *)spi0Esp32RxBuffer.data);
      ESP_LOGI(TAG, "Rx.crc : %04X ", spi0Esp32RxBuffer.crc);
      ESP_LOGI(TAG, "   crc : %04X ", k210Esp32DataCrc16(spi0Esp32RxBuffer));
    } else {
      ESP_LOGI(TAG, "ERR_CODE:%d ", espErr);
    }
    if (ctx >= 10) {
      ctx = 0;
    } else {
      ctx++;
    }
  }
}

K210ESP32Communication::~K210ESP32Communication(void) {
}