#include "AllDevices.h"

/* Devices */
K210 k210("K210");

Device *DEVICES[] = {
    &k210,
};

const uint8_t NUM_OF_DEVICES = ((uint8_t)(sizeof(DEVICES) / sizeof(Device *)));