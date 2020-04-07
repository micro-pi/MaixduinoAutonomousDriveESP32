#ifndef ALL_MODULES_H
#define ALL_MODULES_H

#include "HttpServer/HttpServer.h"
#include "k210_esp32_com/K210ESP32Communication.h"
#include "wifi/WifiModule.h"

/* Modules */
extern WifiModule wifiModule;
extern K210ESP32Communication k210Esp32Communication;
extern HttpServer httpServer;

extern Module *MODULES_10MS[];

extern const uint8_t NUM_OF_MODULES_10MS;

#endif