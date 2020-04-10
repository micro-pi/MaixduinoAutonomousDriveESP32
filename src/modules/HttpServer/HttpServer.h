#ifndef HTTP_MODULE_H
#define HTTP_MODULE_H

#include "../../common/common.h"
#include "../Module.h"
#include <esp_http_server.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>

/* 
 * TODO: Move WIFI functionality to separate class
 * Add handlers to detect wifi connection
 */
class HttpServer : public Module {
private:
  MovingModuleInterface movingModuleInterface;
  xQueueHandle movingModuleCommandsQueue;
  httpd_handle_t server;
  httpd_uri_t indexUri;
  /**
   * SSID of target AP. 
   */
  char ssid[32];
  /**
   * Password of target AP. 
   */
  char password[64];

public:
  HttpServer(const char *moduleName);
  ErrorCode initModule(void);
  void mainFunction(void);
  void setMovingModuleCommandsQueue(xQueueHandle movingModuleCommandsQueue);
  void setSsid(const char *ssid);
  void setPassword(const char *password);
  esp_err_t stopWebserver(void);
  esp_err_t startWebserver(void);
  httpd_handle_t getServer(void);
  esp_err_t indexGetHandler(httpd_req_t *req);
  virtual ~HttpServer();
};

#endif