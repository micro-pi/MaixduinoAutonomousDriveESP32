#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include <driver/spi_slave.h>
#include <string.h>

#include "common/common.h"
#include "project_cfg.h"

#include "devices/AllDevices.h"
#include "modules/AllModules.h"

#include "tasks/tasks.h"

#include "WIFIAP.h"

#define EXAMPLE_ESP_WIFI_SSID "MaixAP"
#define EXAMPLE_ESP_WIFI_PASS "MaixPassword"

static const char *TAG = "MAD_ESP32";

/* Objects */
static xQueueHandle movingModuleCommandsQueue;
static MovingModuleInterface movingModuleInterface;

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

/* An HTTP GET handler */
esp_err_t indexGetHandler(httpd_req_t *req) {
  BaseType_t xStatus;
  char *buf;
  size_t buf_len;

  /* Get header value string length and allocate memory for length + 1, extra byte for null termination */
  buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
  if (buf_len > 1) {
    buf = new char[buf_len];
    /* Copy null terminated value string into buffer */
    if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
      ESP_LOGI(TAG, "Found header => Host: %s", buf);
    }
    delete buf;
  }

  /* Read URL query string length and allocate memory for length + 1, extra byte for null termination */
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = new char[buf_len];
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      ESP_LOGI(TAG, "Found URL query => %s", buf);
      char cmd[32];
      char attr[32];
      char dir[32];
      char pwm[32];
      httpd_resp_set_hdr(req, "Content-Type", HTTPD_TYPE_JSON);
      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
      httpd_resp_sendstr_chunk(req, "{");
      /* Get command */
      if (httpd_query_key_value(buf, "cmd", cmd, sizeof(cmd)) == ESP_OK) {
        if (strcmp(cmd, "hello") == 0) {
          /* Hello message */
          httpd_resp_sendstr_chunk(req, "\"msg\":\"Hello World!\"");
        } else if (strcmp(cmd, "v") == 0) {
          /* Application version */
          httpd_resp_sendstr_chunk(req, "\"version\":\"1.0.0\"");
        } else if (strcmp(cmd, "0") == 0) {
          if (movingModuleCommandsQueue != nullptr) {
            movingModuleInterface.command = MOVING_MODULE_COMMAND_STOP;
            movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
            movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_NONE;
            movingModuleInterface.pwmValue = 0;
            /* Add CMD to xQueue */
            xStatus = xQueueSendToBack(movingModuleCommandsQueue, (void *)&movingModuleInterface, 0);
            /* Print warning message in case if the xQueue is full */
            if (xStatus == pdFAIL) {
              ESP_LOGW(TAG, "Could not send to the CMD queue.");
            }
          }
          /* MOVING_MODULE_COMMAND_STOP */
          httpd_resp_sendstr_chunk(req, "\"cmd\":\"MOVING_MODULE_COMMAND_STOP\"");
        } else if (strcmp(cmd, "1") == 0) {
          if (movingModuleCommandsQueue != nullptr) {
            movingModuleInterface.command = MOVING_MODULE_COMMAND_START;
            movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
            movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_FORWARD;
            movingModuleInterface.pwmValue = (uint16_t)(0.40 * 1000);
            /* Add CMD to xQueue */
            xStatus = xQueueSendToBack(movingModuleCommandsQueue, (void *)&movingModuleInterface, 0);
            /* Print warning message in case if the xQueue is full */
            if (xStatus == pdFAIL) {
              ESP_LOGW(TAG, "Could not send to the CMD queue.");
            }
          }
          /* MOVING_MODULE_COMMAND_START */
          httpd_resp_sendstr_chunk(req, "\"cmd\":\"MOVING_MODULE_COMMAND_START\"");
        } else if (strcmp(cmd, "2") == 0) {
          if (movingModuleCommandsQueue != nullptr) {
            movingModuleInterface.command = MOVING_MODULE_COMMAND_MOVE;

            if (httpd_query_key_value(buf, "attr", attr, sizeof(attr)) == ESP_OK) {
              movingModuleInterface.commandAttribute = static_cast<MovingModuleCommandAttribute>(atoi(attr));
            } else {
              movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_NONE;
            }

            if (httpd_query_key_value(buf, "dir", dir, sizeof(dir)) == ESP_OK) {
              movingModuleInterface.movingDirection = static_cast<MovingModuleDirection>(atoi(dir));
            } else {
              movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_NONE;
            }

            if (httpd_query_key_value(buf, "pwm", pwm, sizeof(pwm)) == ESP_OK) {
              movingModuleInterface.pwmValue = static_cast<uint16_t>(atoi(pwm));
            } else {
              movingModuleInterface.pwmValue = (uint16_t)(0.40 * 1000);
            }
            /* Add CMD to xQueue */
            xStatus = xQueueSendToBack(movingModuleCommandsQueue, (void *)&movingModuleInterface, 0);
            /* Print warning message in case if the xQueue is full */
            if (xStatus == pdFAIL) {
              ESP_LOGW(TAG, "Could not send to the CMD queue.");
            }
          }
          /* MOVING_MODULE_COMMAND_MOVE */
          httpd_resp_sendstr_chunk(req, "\"cmd\":\"MOVING_MODULE_COMMAND_MOVE\"");
        } else if (strcmp(cmd, "3") == 0) {
          if (movingModuleCommandsQueue != nullptr) {
            movingModuleInterface.command = MOVING_MODULE_COMMAND_PWM;
            movingModuleInterface.commandAttribute = MOVING_MODULE_COMMAND_ATTRIBUTE_ALL;
            movingModuleInterface.movingDirection = MOVING_MODULE_DIRECTION_FORWARD;
            movingModuleInterface.pwmValue = (uint16_t)(0.45 * 1000);
            /* Add CMD to xQueue */
            xStatus = xQueueSendToBack(movingModuleCommandsQueue, (void *)&movingModuleInterface, 0);
            /* Print warning message in case if the xQueue is full */
            if (xStatus == pdFAIL) {
              ESP_LOGW(TAG, "Could not send to the CMD queue.");
            }
          }
          /* MOVING_MODULE_COMMAND_PWM */
          httpd_resp_sendstr_chunk(req, "\"cmd\":\"MOVING_MODULE_COMMAND_PWM\"");
        } else {
          /* Invalid command */
          httpd_resp_sendstr_chunk(req, "\"err\":\"invalid cmd\",\"cmd\":\"");
          httpd_resp_sendstr_chunk(req, cmd);
          httpd_resp_sendstr_chunk(req, "\"");
        }
      } else {
        httpd_resp_sendstr_chunk(req, "\"err\":\"unknown param\"");
      }
      httpd_resp_sendstr_chunk(req, "}");
      httpd_resp_sendstr_chunk(req, NULL);
    }
    delete buf;
  }

  /* After sending the HTTP response the old HTTP request headers are lost. Check if HTTP request headers can be read now. */
  if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
    ESP_LOGI(TAG, "Request headers lost");
  }
  return ESP_OK;
}

httpd_uri_t indexUri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = &indexGetHandler,
    .user_ctx = nullptr};

httpd_handle_t start_webserver(void) {
  esp_err_t espErr;
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  /* Start the httpd server */
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  espErr = httpd_start(&server, &config);
  if (espErr == ESP_OK) {
    /* Set URI handlers */
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &indexUri);
    return server;
  }

  ESP_LOGI(TAG, "Error starting server!");
  return NULL;
}

void stop_webserver(httpd_handle_t server) {
  httpd_stop(server);
}

static esp_err_t event_handler(void *ctx, system_event_t *event) {
  httpd_handle_t *server = (httpd_handle_t *)ctx;

  switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
      ESP_ERROR_CHECK(esp_wifi_connect());
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
      ESP_LOGI(TAG, "Got IP: '%s'", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

      if (*server == NULL) {
        *server = start_webserver();
      }
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
      ESP_ERROR_CHECK(esp_wifi_connect());

      if (*server) {
        stop_webserver(*server);
        *server = NULL;
      }
      break;
    default:
      break;
  }
  return ESP_OK;
}

static void initialise_wifi(void *arg) {
  wifi_config_t wifi_config = {};
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, arg));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  strcpy(reinterpret_cast<char *>(wifi_config.sta.ssid), "HUAWEI-13ef");
  strcpy(reinterpret_cast<char *>(wifi_config.sta.password), "ad6803f1");
  ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

extern "C" void app_main() {
  static httpd_handle_t server = NULL;

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

  // wifiAp.begin("HUAWEI-13ef", "ad6803f1");
  initialise_wifi(&server);
  // server = start_webserver();
}
