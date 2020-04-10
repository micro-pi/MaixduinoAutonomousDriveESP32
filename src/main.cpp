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
#include "modules/HttpServer/HttpServer.h"

#include "tasks/tasks.h"

static const char *TAG = "MAD_ESP32";

/* Objects */
static xQueueHandle movingModuleCommandsQueue;

extern "C" void app_main() {
  uint32_t i;
  ErrorCode errorCode;
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

  /* Initialize Devices */
  k210.setHost(RCV_HOST);
  k210.setTicksToWait(portMAX_DELAY);

  ESP_LOGI(TAG, "Devices: %d", NUM_OF_DEVICES);
  for (i = 0; i < NUM_OF_DEVICES; i++) {
    errorCode = DEVICES[i]->begin();
    if (errorCode == E_OK) {
      ESP_LOGI(TAG, "Device \"%s\" initialized successfully!", DEVICES[i]->getName());
    } else {
      ESP_LOGW(TAG, "Device \"%s\" not initialized!", DEVICES[i]->getName());
    }
  }

  /* Initialize Communication Module */
  k210Esp32Communication.setMovingModuleCommandsQueue(movingModuleCommandsQueue);
  k210Esp32Communication.setK210Device(k210);
  k210Esp32Communication.init();
  ESP_LOGI(TAG, "Run task %s", "k210Esp32CommunicationTask");
  xReturn = xTaskCreatePinnedToCore(&k210Esp32CommunicationTask, "k210Esp32CommunicationTask", 2048, NULL, 5, NULL, CORE_0);
  if (xReturn != pdPASS) {
    ESP_LOGI(TAG, "Task %s run problem", "k210Esp32CommunicationTask");
  } else {
    ESP_LOGI(TAG, "Rask %s is running", "k210Esp32CommunicationTask");
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
  httpServer.setSsid("internet");
  httpServer.setPassword("internet_wifi");
  httpServer.init();
}
