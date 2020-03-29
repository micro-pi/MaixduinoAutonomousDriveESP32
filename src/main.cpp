#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include <driver/spi_slave.h>
#include <string.h>

#include "common/common.h"
#include "project_cfg.h"

#include "devices/K210/K210.h"

static const char *TAG = "MAD_ESP32";

static K210 k210("K210");

DMA_ATTR K210ESP32Data spi0Esp32TxBuffer;
DMA_ATTR K210ESP32Data spi0Esp32RxBuffer;

void blinkTaskFunction(void *parameter) {
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void adcTaskFunction(void *parameter) {
  while (1) {
    vTaskDelay(1500 / portTICK_PERIOD_MS);
  }
}

/* https://www.esp32.com/viewtopic.php?t=6512 */
void spiTaskFunction(void *parameter) {
  const TickType_t xFrequency = 10;
  TickType_t xLastWakeTime;
  esp_err_t espErr;
  uint8_t ctx = 0;
  MovingModuleInterface movingModuleInterface;

  /* Initialise the xLastWakeTime variable with the current time. */
  xLastWakeTime = xTaskGetTickCount();

  ESP_LOGI("spi_slave_task", "");

  while (true) {
    if (ctx % 2 == 0) {
      movingModuleInterface.command = MOVING_MODULE_COMMAND_PWM;
      movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
      movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_NONE;
      movingModuleInterface.pwmValue = 100;

      memcpy(spi0Esp32TxBuffer.data, &movingModuleInterface, sizeof(MovingModuleInterface));
      spi0Esp32TxBuffer.type = MOVING_CMD;
      spi0Esp32TxBuffer.id = (uint8_t)xLastWakeTime;
      spi0Esp32TxBuffer.size = sizeof(MovingModuleInterface);
    } else {
      sprintf((char *)spi0Esp32TxBuffer.data, "Hello K210, xLastWakeTime: %d", xLastWakeTime);
      spi0Esp32TxBuffer.type = STRING;
      spi0Esp32TxBuffer.id = (uint8_t)xLastWakeTime;
      spi0Esp32TxBuffer.size = strlen((char *)spi0Esp32TxBuffer.data);
    }

    espErr = k210.transferFullDuplex(spi0Esp32TxBuffer, spi0Esp32RxBuffer);
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
    ctx++;

    /* Wait for the next cycle. */
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(xFrequency));
  }
}

static esp_err_t spiSlaveInit(void) {
  esp_err_t ret;

  /* Configuration for the SPI bus */
  spi_bus_config_t busConfig;
  /* Configuration for the SPI slave interface */
  spi_slave_interface_config_t slaveInterfaceConfig;

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

  return ret;
}

extern "C" void app_main() {
  esp_err_t ret;

  /* Initialize the default NVS partition. */
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  }

  ESP_LOGI(TAG, "Start blink task");
  xTaskCreatePinnedToCore(&blinkTaskFunction, "blink_task", 2048, NULL, 5, NULL, 0);
  ESP_LOGI(TAG, "Start ADC task");
  xTaskCreatePinnedToCore(&adcTaskFunction, "adc_task", 2048, NULL, 5, NULL, 1);

  ret = spiSlaveInit();
  if (ret == ESP_OK) {
    k210.setHost(RCV_HOST);
    k210.setTicksToWait(portMAX_DELAY);
    k210.begin();

    ESP_LOGI(TAG, "Start SPI task");
    xTaskCreatePinnedToCore(&spiTaskFunction, "spi_task", 2048, NULL, 5, NULL, 1);
  }
}
