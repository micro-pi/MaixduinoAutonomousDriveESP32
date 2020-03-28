#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define SPI_BUF_SIZE (64)

typedef enum {
  EMPTY,
  STRING,
  CMD,
  BYTES
} K210ESP32DataType;

#define BUFFER_SIZE (SPI_BUF_SIZE - 2 * sizeof(uint8_t) - sizeof(K210ESP32DataType) - sizeof(uint16_t))

typedef struct {
  uint8_t data[BUFFER_SIZE];
  K210ESP32DataType type;
  uint8_t id;
  uint8_t size;
  uint16_t crc;
} K210ESP32Data;

typedef enum {
  E_OK,
  E_NOK
} ErrorCode;

/**
 * Moving Module Commands
 */
typedef enum {
  /**
   * Stop command
   */
  MOVING_MODULE_COMMAND_STOP = (0u),
  /**
   * Start command
   */
  MOVING_MODULE_COMMAND_START = (1u),
  /**
   * Move command
   */
  MOVING_MODULE_COMMAND_MOVE = (2u),
  /**
   * PWM command
   */
  MOVING_MODULE_COMMAND_PWM = (3u),
  /**
   * None
   */
  MOVING_MODULE_COMMAND_NONE
} MovingModuleCommands;

typedef enum {
  /**
   * All
   */
  MOVING_MODULE_COMMAND_ATTRIBUTE_ALL = (0u),
  /**
   * Right
   */
  MOVING_MODULE_COMMAND_ATTRIBUTE_RIGHT = (1u),
  /**
   * Left
   */
  MOVING_MODULE_COMMAND_ATTRIBUTE_LEFT = (2u),
  /**
   * None
   */
  MOVING_MODULE_COMMAND_ATTRIBUTE_NONE
} MovingModuleCommandAttribute;

typedef enum {
  /**
   * Forward command
   */
  MOVING_MODULE_DIRECTION_FORWARD = (0u),
  /**
   * Back command
   */
  MOVING_MODULE_DIRECTION_BACK = (1u),
  /**
   * Around command
   */
  MOVING_MODULE_DIRECTION_AROUND = (2u),
  /**
   * None
   */
  MOVING_MODULE_DIRECTION_NONE
} MovingModuleDirection;

typedef struct {
  MovingModuleCommands command;
  MovingModuleCommandAttribute commandAttribute;
  MovingModuleDirection movingDirection;
  uint16_t pwmValue;
} MovingModuleInterface;

/***** crc16 *****/
//Tested
#define CRC16_DNP 0x3D65   // DNP, IEC 870, M-BUS, wM-BUS, ...
#define CRC16_CCITT 0x1021 // X.25, V.41, HDLC FCS, Bluetooth, ...

//Other polynoms not tested
#define CRC16_IBM 0x8005     // ModBus, USB, Bisync, CRC-16, CRC-16-ANSI, ...
#define CRC16_T10_DIF 0x8BB7 // SCSI DIF
#define CRC16_DECT 0x0589    // Cordeless Telephones
#define CRC16_ARINC 0xA02B   // ACARS Aplications

#define POLYNOM CRC16_DNP // Define the used polynom from one of the aboves

/**
 *  It calculates the new crc16 with the newByte. Variable crcValue is the actual or initial value (0). 
 */
uint16_t crc16(uint16_t crcValue, uint8_t newByte);

uint16_t arrayCrc16(uint8_t *data, uint8_t len);

uint16_t k210Esp32DataCrc16(K210ESP32Data &k210Esp32Data);

#endif
