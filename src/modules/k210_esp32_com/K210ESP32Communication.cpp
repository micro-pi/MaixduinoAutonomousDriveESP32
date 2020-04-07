#include "K210ESP32Communication.h"

#include <driver/gpio.h>
#include <driver/spi_slave.h>
#include <esp_log.h>
#include <string.h>

static const char *TAG = "KECM_ESP32";

K210ESP32Communication::K210ESP32Communication(const char *moduleName) : Module(moduleName) {
  this->movingModuleCommandsQueue = nullptr;
  this->k210 = nullptr;
}

ErrorCode K210ESP32Communication::initModule(void) {
  ErrorCode errorCode;
  esp_err_t ret;

  busConfig.mosi_io_num = SPI_SLAVE_MOSI_PIN;
  busConfig.miso_io_num = SPI_SLAVE_MISO_PIN;
  busConfig.sclk_io_num = SPI_SLAVE_SCLK_PIN;
  busConfig.quadwp_io_num = -1;
  busConfig.quadhd_io_num = -1;
  busConfig.max_transfer_sz = 0;
  busConfig.flags = 0;
  busConfig.intr_flags = 0;

  slaveInterfaceConfig.spics_io_num = SPI_SLAVE_CS_PIN;
  slaveInterfaceConfig.flags = 0;
  slaveInterfaceConfig.queue_size = INTERNAL_QSIZE;
  slaveInterfaceConfig.mode = 2;
  slaveInterfaceConfig.post_setup_cb = nullptr;
  slaveInterfaceConfig.post_trans_cb = nullptr;

  /* Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected. */
  gpio_set_pull_mode(SPI_SLAVE_MOSI_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(SPI_SLAVE_SCLK_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(SPI_SLAVE_CS_PIN, GPIO_PULLUP_ONLY);

  /* Initialize SPI slave interface */
  ret = spi_slave_initialize(RCV_HOST, &busConfig, &slaveInterfaceConfig, DMA_CHAN);
  if (ret == ESP_OK) {
    errorCode = E_OK;
  } else {
    errorCode = E_NOK;
  }

  return errorCode;
}

void K210ESP32Communication::setMovingModuleCommandsQueue(xQueueHandle movingModuleCommandsQueue) {
  this->movingModuleCommandsQueue = movingModuleCommandsQueue;
}

void K210ESP32Communication::setK210Device(K210 &k210) {
  this->k210 = &k210;
}

void K210ESP32Communication::mainFunction(void) {
  BaseType_t xStatus;
  esp_err_t espErr;
  uint16_t crc;

  if ((getErrorCode() == E_OK) && (k210 != nullptr)) {
    /* TODO: Create xQueue of tx messages (CMD's, Messages etc.) */
    // if (ctx == 2) {
    //   movingModuleInterface.command = MOVING_MODULE_COMMAND_MOVE;
    //   movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
    //   movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_FORWARD;
    //   movingModuleInterface.pwmValue = (uint16_t)(0.45 * 1000);

    //   memcpy(spi0Esp32TxBuffer.data, &movingModuleInterface, sizeof(MovingModuleInterface));
    //   spi0Esp32TxBuffer.type = MOVING_CMD;
    //   spi0Esp32TxBuffer.id = (uint8_t)1;
    //   spi0Esp32TxBuffer.size = sizeof(MovingModuleInterface);
    // } else if (ctx == 5) {
    //   movingModuleInterface.command = MOVING_MODULE_COMMAND_PWM;
    //   movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
    //   movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_NONE;
    //   movingModuleInterface.pwmValue = (uint16_t)(0.40 * 1000);

    //   memcpy(spi0Esp32TxBuffer.data, &movingModuleInterface, sizeof(MovingModuleInterface));
    //   spi0Esp32TxBuffer.type = MOVING_CMD;
    //   spi0Esp32TxBuffer.id = (uint8_t)1;
    //   spi0Esp32TxBuffer.size = sizeof(MovingModuleInterface);
    // } else if (ctx == 10) {
    //   movingModuleInterface.command = MOVING_MODULE_COMMAND_STOP;
    //   movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
    //   movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_NONE;
    //   movingModuleInterface.pwmValue = 0;

    //   memcpy(spi0Esp32TxBuffer.data, &movingModuleInterface, sizeof(MovingModuleInterface));
    //   spi0Esp32TxBuffer.type = MOVING_CMD;
    //   spi0Esp32TxBuffer.id = (uint8_t)1;
    //   spi0Esp32TxBuffer.size = sizeof(MovingModuleInterface);
    // } else {
    // }

    xStatus = xQueueReceive(this->movingModuleCommandsQueue, &movingModuleInterface, 0);
    if (xStatus == pdFAIL) {
      memset(spi0Esp32TxBuffer.data, 0, sizeof(spi0Esp32TxBuffer.data));
      spi0Esp32TxBuffer.type = EMPTY;
      spi0Esp32TxBuffer.id = (uint8_t)1;
      spi0Esp32TxBuffer.size = 0;
    } else {
      /* Set CMD to data array */
      memcpy(spi0Esp32TxBuffer.data, &movingModuleInterface, sizeof(MovingModuleInterface));
      spi0Esp32TxBuffer.type = MOVING_CMD;
      spi0Esp32TxBuffer.id = (uint8_t)1;
      spi0Esp32TxBuffer.size = sizeof(MovingModuleInterface);
    }

    espErr = k210->transferFullDuplex(spi0Esp32TxBuffer, spi0Esp32RxBuffer);
    if (espErr == ESP_OK) {
      crc = k210Esp32DataCrc16(spi0Esp32RxBuffer);
      if (spi0Esp32RxBuffer.crc == crc) {
        switch (spi0Esp32RxBuffer.type) {
          case EMPTY:
            break;

          case STRING:
            /* Used for tests */
            // ESP_LOGI(TAG, "Rx.id  : %d ", spi0Esp32RxBuffer.id);
            // ESP_LOGI(TAG, "Rx.type: %d ", spi0Esp32RxBuffer.type);
            // ESP_LOGI(TAG, "Rx.size: %d ", spi0Esp32RxBuffer.size);
            // ESP_LOGI(TAG, "Rx.data: %s ", (char *)spi0Esp32RxBuffer.data);
            // ESP_LOGI(TAG, "Rx.crc : %04X ", spi0Esp32RxBuffer.crc);
            // ESP_LOGI(TAG, "   crc : %04X ", crc);
            break;

          case MOVING_CMD:
          case BYTES:
            /* code */
            break;

          default:
            ESP_LOGW(TAG, "Unknown type '%d'", spi0Esp32RxBuffer.type);
            break;
        }
      } else {
        ESP_LOGW(TAG, "CRC ERR: RX(%d) != CRC(%d)", spi0Esp32RxBuffer.crc, crc);
      }
    } else {
      ESP_LOGW(TAG, "ERR_CODE:%d ", espErr);
    }
  }
}

K210ESP32Communication::~K210ESP32Communication(void) {
}