#ifndef PROJECT_CFG_H
#define PROJECT_CFG_H

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>

/* SPI3 Pins */
/** 
 * SPI0_MOSI -> ESP32_14 
 */
#define SPI_SLAVE_MOSI_PIN (GPIO_NUM_14)
/**
 *  SPI0_MISO -> ESP32_23
 */
#define SPI_SLAVE_MISO_PIN (GPIO_NUM_23)
/** 
 * SPI0_SCLK -> ESP32_18 
 */
#define SPI_SLAVE_SCLK_PIN (GPIO_NUM_18)
/** 
 * IO25 -> ESP32_SPI_CS(5) 
 */
#define SPI_SLAVE_CS_PIN (GPIO_NUM_5)

#ifdef CONFIG_IDF_TARGET_ESP32
#define RCV_HOST VSPI_HOST
#define DMA_CHAN (2)

#elif defined CONFIG_IDF_TARGET_ESP32S2
#define RCV_HOST SPI3_HOST
#define DMA_CHAN RCV_HOST

#endif

#define INTERNAL_QSIZE (32)
#define SPI_PKT_SOF (0x55AA)

#define CORE_0 (0)
#define CORE_1 (1)

#endif
