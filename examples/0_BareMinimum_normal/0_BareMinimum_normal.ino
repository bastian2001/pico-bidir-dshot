/**
 * This is a bare minimum example for bidirectional DShot using this library.
 * It sends a throttle value of 0 to the ESC every 200ish microseconds using DShot600.
 * This will not spin the motor, but you should hear the full beep sequence.
 */

#include <PIO_DShot.h>

#define PIN_BASE 10
#define PIN_COUNT 4
// you can choose up to 4 motors per DShotX4 instance. The pins are assigned in order, starting from PIN_BASE.
// In this case PIN_BASE = 10 and PIN_COUNT = 4. This will use pins 10, 11, 12 and 13.
// Arbitrary pin numbers are not supported by PIO, but you can use multiple instances of DShotX4 to use different pins.
// setting a lower PIN_COUNT will leave the other pins untouched.

DShotX4 *esc;

void setup() {
	// initialize the ESC. This cannot be done globally (needs to know the clock speed). Your MCU will crash if you try.
	esc = new DShotX4(PIN_BASE, PIN_COUNT);
	// esc = new DShotX4(PIN_BASE, PIN_COUNT, 600, pio0, -1); // -> optional parameters: speed, pio, sm (-1 = autodetect)
}

void loop() {
	// sendThrottles is non-blocking. That means, we must not send it too fast (manual delayMicroseconds).
	// at DShot600, the maximum theoretical loop rate is about 35kHz, but it is unlikely that any ESC will use all those values, if it even works. Here we use roughly 5kHz.
	delayMicroseconds(200);

	uint16_t throttles[4] = {0, 0, 0, 0}; // throttle values for each motor. Always hand over an array of 4 values, even if you only use one motor. The other values will be ignored.
	esc->sendThrottles(throttles); // 0-2000

	// try setting PIN_COUNT to 1, 2 or 3 and see what happens. You should hear only 1, 2 or 3 motors beeping.
}
