#ifndef WIFIAP_H
#define WIFIAP_H

#include <esp_wifi.h>
#include <stdint.h>

class WIFIAP {
public:
  bool begin(const char *ssid, const char *passphrase = nullptr, int32_t channel = 0, const uint8_t *bssid = nullptr, bool connect = true);
  bool softAP(const char *ssid, const char *passphrase = nullptr, uint8_t channel = 1, uint8_t ssidHidden = 0, uint8_t maxConnection = 4);

  wifi_mode_t getMode(void);
  ip4_addr_t getSoftAPIP(void);
};

#endif