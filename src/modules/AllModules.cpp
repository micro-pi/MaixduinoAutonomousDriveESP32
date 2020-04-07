#include "AllModules.h"

/* Modules */
WifiModule wifiModule("WIFI Module");
K210ESP32Communication k210Esp32Communication("K210/ESP32 Communication");
HttpServer httpServer("HTTP Server");

Module *MODULES_10MS[] = {
    &wifiModule,
};

const uint8_t NUM_OF_MODULES_10MS = ((uint8_t)(sizeof(MODULES_10MS) / sizeof(Module *)));