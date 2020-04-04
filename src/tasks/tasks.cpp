#include "tasks.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "devices/AllDevices.h"
#include "modules/AllModules.h"

static const char *TAG = "TASKS_ESP32";

void task10ms(void *parameter) {
  const TickType_t xFrequency = 10;
  TickType_t xLastWakeTime;
  uint32_t i;
  /* Initialise the xLastWakeTime variable with the current time. */
  xLastWakeTime = xTaskGetTickCount();

  while (1) {
    for (i = 0; i < NUM_OF_MODULES_10MS; i++) {
      MODULES_10MS[i]->mainFunction();
    }

    /* Wait for the next cycle. */
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(xFrequency));
  }
}

/* https://www.esp32.com/viewtopic.php?t=6512 */
void k210Esp32CommunicationTask(void *parameter) {
  const TickType_t xFrequency = 10;
  TickType_t xLastWakeTime;

  /* Initialise the xLastWakeTime variable with the current time. */
  xLastWakeTime = xTaskGetTickCount();

  while (true) {
    k210Esp32Communication.mainFunction();

    /* Wait for the next cycle. */
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(xFrequency));
  }
}