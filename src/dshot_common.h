#include "hardware/pio.h"

#define DBG defined(DSHOT_DEBUG)

#if DBG
#include "Arduino.h"

#define DEBUG_PRINTF(x, ...)                         \
	Serial.printf("%15s:%3d: ", __FILE__, __LINE__); \
	Serial.printf(x, __VA_ARGS__)

void pioToPioStr(PIO pio, char str[32]);
#else
#define DEBUG_PRINTF
#endif