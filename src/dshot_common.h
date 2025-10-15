#include "dshot_config.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#define DBG defined(DSHOT_DEBUG)

// Pico-SDK: doesn't have ARDUINO defined
// Earle Phil Hower's core: has ARDUINO defined, but also a version
// Arduino standard core: needs special treatment, only has ARDUINO defined
#define CAN_USE_SDK_FUNCTIONS ((defined(ARDUINO) && defined(ARDUINO_PICO_MAJOR)) || !defined(ARDUINO))

void gpio_init(uint gpio);

#if DBG
#include "Arduino.h"

#define DEBUG_PRINTF(x, ...)                         \
	Serial.printf("%15s:%3d: ", __FILE__, __LINE__); \
	Serial.printf(x, __VA_ARGS__)

void pioToPioStr(PIO pio, char str[32]);
#else
#define DEBUG_PRINTF
#endif
