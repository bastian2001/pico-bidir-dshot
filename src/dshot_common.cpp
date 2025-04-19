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