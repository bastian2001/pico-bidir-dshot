/**
 * This is a bare minimum example for bidirectional DShot using this library.
 * It sends a throttle value of 0 to the ESC every 200ish microseconds using DShot600.
 * This will not spin the motor, but you should hear the full beep sequence.
 */

#include <PIO_DShot.h>

#define PIN 10

BidirDShotX1 *esc;

void setup() {
	// initialize the ESC. This cannot be done globally (needs to know the clock speed). Your MCU will crash if you try.
	esc = new BidirDShotX1(PIN);
	// esc = new BidirDShotX1(PIN, 600, pio0, -1); // -> optional parameters: speed, pio, sm (-1 = autodetect)
}

void loop() {
	// sendThrottle is non-blocking. That means, we must not send it too fast (manual delayMicroseconds).
	// at DShot600, the maximum achievable loop rate is about 9kHz. Roughly 6k for DShot300, 12k for DShot1200.
	delayMicroseconds(200);
	esc->sendThrottle(0);
}
