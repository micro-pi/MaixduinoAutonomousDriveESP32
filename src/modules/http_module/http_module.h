#ifndef HTTP_MODULE_H
#define HTTP_MODULE_H

#include "../../common/common.h"
#include <esp_http_server.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>

class HttpServer {
private:
  MovingModuleInterface movingModuleInterface;
  xQueueHandle movingModuleCommandsQueue;
  httpd_handle_t server;
  httpd_uri_t indexUri;

public:
  HttpServer();
  void setMovingModuleCommandsQueue(xQueueHandle movingModuleCommandsQueue);
  void initialiseWifi(const char *ssid, const char *password);
  esp_err_t stopWebserver(void);
  esp_err_t startWebserver(void);
  httpd_handle_t getServer(void);
  esp_err_t indexGetHandler(httpd_req_t *req);
  virtual ~HttpServer();
};

#endif