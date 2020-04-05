#include <WIFIAP.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <string.h>

static const char *TAG = "WIFIAP";

static esp_err_t eventHandler(void *ctx, system_event_t *event);

bool WIFIAP::begin(const char *ssid, const char *passphrase, int32_t channel, const uint8_t *bssid, bool connect) {
  wifi_config_t wifiConfig = {};
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  if ((ssid == nullptr) || (ssid[0] == '\0') || strlen(ssid) > 31) {
    /* fail SSID missing */
    ESP_LOGE(TAG, "SSID too long or missing!");
    return false;
  }
  if ((passphrase != nullptr) && (strlen(passphrase) > 64)) {
    /* fail passphrase too long */
    ESP_LOGE(TAG, "passphrase too long!");
    return false;
  }

  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(eventHandler, nullptr));
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  strcpy(reinterpret_cast<char *>(wifiConfig.sta.ssid), ssid);
  if (passphrase) {
    if (strlen(passphrase) == 64) {
      /* it's not a passphrase, is the PSK */
      memcpy(reinterpret_cast<char *>(wifiConfig.sta.password), passphrase, 64);
    } else {
      strcpy(reinterpret_cast<char *>(wifiConfig.sta.password), passphrase);
    }
  }
  if (bssid) {
    wifiConfig.sta.bssid_set = 1;
    memcpy((void *)&wifiConfig.sta.bssid[0], (void *)bssid, 6);
  }
  if (channel > 0 && channel <= 13) {
    wifiConfig.sta.channel = channel;
  }
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig));
  ESP_ERROR_CHECK(esp_wifi_start());

  return true;
}

bool WIFIAP::softAP(const char *ssid, const char *passphrase, uint8_t channel, uint8_t ssidHidden, uint8_t maxConnection) {
  wifi_config_t wifiConfig = {};
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  if ((ssid == nullptr) || (ssid[0] == '\0')) {
    /* fail SSID missing */
    ESP_LOGE(TAG, "SSID missing!");
    return false;
  }

  if ((passphrase != nullptr) && ((strlen(passphrase) > 0) && (strlen(passphrase) < 8))) {
    /* fail passphrase too short */
    ESP_LOGE(TAG, "passphrase too short!");
    return false;
  }

  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(eventHandler, nullptr));
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  strlcpy(reinterpret_cast<char *>(wifiConfig.ap.ssid), ssid, sizeof(wifiConfig.ap.ssid));
  wifiConfig.ap.channel = channel;
  wifiConfig.ap.ssid_len = strlen(reinterpret_cast<char *>(wifiConfig.ap.ssid));
  wifiConfig.ap.ssid_hidden = ssidHidden;
  wifiConfig.ap.max_connection = maxConnection;
  wifiConfig.ap.beacon_interval = 100;
  if ((passphrase == nullptr) || (strlen(passphrase) == 0)) {
    wifiConfig.ap.authmode = WIFI_AUTH_OPEN;
    wifiConfig.ap.password[0] = '\0';
  } else {
    wifiConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;
    strlcpy(reinterpret_cast<char *>(wifiConfig.ap.password), passphrase, sizeof(wifiConfig.ap.password));
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifiConfig));
  ESP_ERROR_CHECK(esp_wifi_start());

  return true;
}

wifi_mode_t WIFIAP::getMode(void) {
  wifi_mode_t mode;
  if (esp_wifi_get_mode(&mode) == ESP_ERR_WIFI_NOT_INIT) {
    ESP_LOGW(TAG, "WiFi not started");
    return WIFI_MODE_NULL;
  }
  return mode;
}

/**
 * Get the softAP interface IP address.
 * @return softAP IP
 */
ip4_addr_t WIFIAP::getSoftAPIP(void) {
  tcpip_adapter_ip_info_t ip;
  if (getMode() == WIFI_MODE_NULL) {
    return ip.ip;
  }
  tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip);
  return ip.ip;
}

static esp_err_t eventHandler(void *ctx, system_event_t *event) {
  switch (event->event_id) {
    case SYSTEM_EVENT_AP_STACONNECTED:
      ESP_LOGI(TAG, "station:" MACSTR " join, AID=%d", MAC2STR(event->event_info.sta_connected.mac), event->event_info.sta_connected.aid);
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      ESP_LOGI(TAG, "station:" MACSTR "leave, AID=%d", MAC2STR(event->event_info.sta_disconnected.mac), event->event_info.sta_disconnected.aid);
      break;

    case SYSTEM_EVENT_STA_START:
      ESP_ERROR_CHECK(esp_wifi_connect());
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG, "Got IP: '%s'", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      ESP_ERROR_CHECK(esp_wifi_connect());
      break;

    default:
      ESP_LOGI(TAG, "------------------------------------");
      break;
  }
  return ESP_OK;
}