#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include <driver/spi_slave.h>
#include <string.h>

#include "common/common.h"
#include "project_cfg.h"

#include "devices/AllDevices.h"
#include "modules/AllModules.h"
#include "modules/http_module/http_module.h"

#include "tasks/tasks.h"

#include "WIFIAP.h"

static const char *TAG = "MAD_ESP32";

/* Objects */
static xQueueHandle movingModuleCommandsQueue;
static HttpServer httpServer;
// static WIFIAP wifiAp;

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

  uint32_t i;
  esp_err_t retSpi;
  esp_err_t retNvs;
  BaseType_t xReturn;

  /* Initialize the default NVS partition. */
  retNvs = nvs_flash_init();
  if (retNvs == ESP_ERR_NVS_NO_FREE_PAGES || retNvs == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  }

  /* Initialize Objects */
  movingModuleCommandsQueue = xQueueCreate(16, sizeof(MovingModuleInterface));

  /* Initialize Ports */
  retSpi = spiSlaveInit();

  /* Initialize Devices */
  k210.setHost(RCV_HOST);
  k210.setTicksToWait(portMAX_DELAY);

  ESP_LOGI(TAG, "Devices: %d", NUM_OF_DEVICES);
  for (i = 0; i < NUM_OF_DEVICES; i++) {
    DEVICES[i]->begin();
  }

  /* Initialize Communication Module */
  k210Esp32Communication.setMovingModuleCommandsQueue(movingModuleCommandsQueue);
  k210Esp32Communication.setK210Device(k210);
  if (retSpi == ESP_OK) {
    ESP_LOGI(TAG, "Run task %s", "k210Esp32CommunicationTask");
    xReturn = xTaskCreatePinnedToCore(&k210Esp32CommunicationTask, "k210Esp32CommunicationTask", 2048, NULL, 5, NULL, CORE_0);
    if (xReturn != pdPASS) {
      ESP_LOGI(TAG, "Task %s run problem", "k210Esp32CommunicationTask");
    } else {
      ESP_LOGI(TAG, "Rask %s is running", "k210Esp32CommunicationTask");
    }
  }

  /* Initialize Modules 10ms */

  ESP_LOGI(TAG, "Modules 10ms: %d", NUM_OF_MODULES_10MS);
  for (i = 0; i < NUM_OF_MODULES_10MS; i++) {
    ESP_LOGI(TAG, "Init module '%s'", MODULES_10MS[i]->getName());
    MODULES_10MS[i]->init();
  }

  ESP_LOGI(TAG, "Run task %s", "task10ms");
  xReturn = xTaskCreatePinnedToCore(&task10ms, "task10ms", 2048, NULL, 5, NULL, CORE_1);
  if (xReturn != pdPASS) {
    ESP_LOGI(TAG, "Task %s run problem", "task10ms");
  } else {
    ESP_LOGI(TAG, "Rask %s is running", "task10ms");
  }

  httpServer.setMovingModuleCommandsQueue(movingModuleCommandsQueue);
  httpServer.initialiseWifi("internet", "internet_wifi");
}
