#include "dshot_common.h"

#if DBG
void pioToPioStr(PIO pio, char str[32]) {
	if (pio == pio0) {
		strcpy(str, "pio0");
	} else if (pio == pio1) {
		strcpy(str, "pio1");
#if NUM_PIOS > 2
	} else if (pio == pio2) {
		strcpy(str, "pio2");
#endif
	} else {
		sprintf(str, "%p (invalid PIO)", pio);
	}
}
#endif

// in case _gpio_init does not exist, define it weakly
extern "C" void _gpio_init(uint gpio) __attribute__((weak));

extern "C" __attribute__((weak)) void gpio_init(uint gpio) {
	_gpio_init(gpio);
}
