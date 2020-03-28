#include "common.h"

/***** crc16.c *****/

uint16_t crc16(uint16_t crcValue, uint8_t newByte) {
  uint8_t i;
  for (i = 0; i < 8; i++) {
    if (((crcValue & 0x8000) >> 8) ^ (newByte & 0x80)) {
      crcValue = (crcValue << 1) ^ POLYNOM;
    } else {
      crcValue = (crcValue << 1);
    }
    newByte <<= 1;
  }
  return crcValue;
}

/***** EXAMPLE *****/

uint16_t arrayCrc16(uint8_t *data, uint16_t len) {
  uint16_t crc;
  uint16_t i;
  /* Initialization of crc to 0x0000 for DNP */
  crc = 0x0000;
  /* Initialization of crc to 0xFFFF for CCITT */
  //crc = 0xFFFF;
  for (i = 0; i < len; i++) {
    crc = crc16(crc, data[i]);
  }
  /* The crc value for DNP it is obtained by NOT operation */
  return (~crc);
  /* The crc value for CCITT */
  //return crc;
}

uint16_t k210Esp32DataCrc16(K210ESP32Data &k210Esp32Data) {
  uint16_t crc;
  uint16_t i;
  uint16_t n;
  uint8_t *p;

  /* Initialization of crc to 0x0000 for DNP */
  crc = 0x0000;

  /* uint8_t data[BUFFER_SIZE]; */
  p = k210Esp32Data.data;
  n = sizeof(k210Esp32Data.data);
  for (i = 0; i < n; i++) {
    crc = crc16(crc, p[i]);
  }

  /* K210ESP32DataType type; */
  p = (uint8_t *)&k210Esp32Data.type;
  n = sizeof(k210Esp32Data.type);
  for (i = 0; i < n; i++) {
    crc = crc16(crc, p[i]);
  }

  /* uint8_t id; */
  p = (uint8_t *)&k210Esp32Data.id;
  n = sizeof(k210Esp32Data.id);
  for (i = 0; i < n; i++) {
    crc = crc16(crc, p[i]);
  }

  /* uint8_t size; */
  p = (uint8_t *)&k210Esp32Data.size;
  n = sizeof(k210Esp32Data.size);
  for (i = 0; i < n; i++) {
    crc = crc16(crc, p[i]);
  }

  /* The crc value for DNP it is obtained by NOT operation */
  return (~crc);
}