#include "http_module.h"
#include <esp_log.h>

#define EXAMPLE_ESP_WIFI_SSID "MaixAP"
#define EXAMPLE_ESP_WIFI_PASS "MaixPassword"

static const char *TAG = "HTTP_ESP32";

static esp_err_t event_handler(void *ctx, system_event_t *event);
static esp_err_t indexGetHandlerFunction(httpd_req_t *req);

HttpServer::HttpServer() {
  this->movingModuleCommandsQueue = nullptr;
  this->server = nullptr;
  indexUri = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = &indexGetHandlerFunction,
      .user_ctx = (void *)this};
}

void HttpServer::setMovingModuleCommandsQueue(xQueueHandle movingModuleCommandsQueue) {
  this->movingModuleCommandsQueue = movingModuleCommandsQueue;
}

void HttpServer::initialiseWifi(const char *ssid, const char *password) {
  wifi_config_t wifi_config = {};
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, (void *)this));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  strcpy(reinterpret_cast<char *>(wifi_config.sta.ssid), ssid);
  strcpy(reinterpret_cast<char *>(wifi_config.sta.password), password);
  ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t HttpServer::stopWebserver(void) {
  esp_err_t espErr;
  espErr = httpd_stop(this->server);
  this->server = NULL;
  return espErr;
}

esp_err_t HttpServer::startWebserver(void) {
  esp_err_t espErr;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  /* Start the httpd server */
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  espErr = httpd_start(&server, &config);
  if (espErr == ESP_OK) {
    /* Set URI handlers */
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &indexUri);
  } else {
    /* MISRA */
  }
  return espErr;
}

httpd_handle_t HttpServer::getServer(void) {
  return this->server;
}

esp_err_t HttpServer::indexGetHandler(httpd_req_t *req) {
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

HttpServer::~HttpServer() {
}

/* An HTTP GET handler */
static esp_err_t indexGetHandlerFunction(httpd_req_t *req) {
  HttpServer *httpServer = (HttpServer *)req->user_ctx;
  esp_err_t espErr;

  if (httpServer != nullptr) {
    espErr = httpServer->indexGetHandler(req);
  } else {
    espErr = ESP_FAIL;
  }

  return espErr;
}

static esp_err_t event_handler(void *ctx, system_event_t *event) {
  HttpServer *httpServer = (HttpServer *)ctx;
  esp_err_t espErr;

  switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
      ESP_ERROR_CHECK(esp_wifi_connect());
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
      ESP_LOGI(TAG, "Got IP: '%s'", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

      if (httpServer != nullptr && httpServer->getServer() == nullptr) {
        espErr = httpServer->startWebserver();
        if (espErr == ESP_OK) {
          ESP_LOGI(TAG, "Server started successfully!");
        } else {
          ESP_LOGW(TAG, "Error starting server!");
        }
      }
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
      ESP_ERROR_CHECK(esp_wifi_connect());

      if (httpServer != nullptr && httpServer->getServer() != NULL) {
        httpServer->stopWebserver();
      }
      break;
    default:
      break;
  }
  return ESP_OK;
}
