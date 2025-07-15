/**
 * This is the equivalent of the standard blink sketch for an ESC.
 * It will pulse the ESC with a throttle of 0% / 20%, changing every 5s.
 * The value is sent roughly with a 5kHz loop rate.
 * Depending on the duration of your startup melody, the last beep may be delayed until the throttle reaches 0%.
 */

#include <PIO_DShot.h>

#define PIN 10

BidirDShotX1 *esc;

void setup() {
	// initialize the ESC. This cannot be done globally (needs to know the clock speed). Your MCU will crash if you try.
	esc = new BidirDShotX1(PIN);
	// esc = new BidirDShotX1(PIN, 600, pio0, -1); -> optional parameters: speed, pio, sm (-1 = autodetect)
}

void loop() {
	// sendThrottle is non-blocking. That means, we must not send it too fast (manual delayMicroseconds).
	// at DShot600, the maximum achievable loop rate is about 9kHz. Roughly 6k for DShot300, 12k for DShot1200.
	delayMicroseconds(200);

	uint16_t throttle = 0;
	if (millis() % 10000 > 5000) {
		throttle = 400; // maximum throttle is 2000, 400 is 20%
	}
	esc->sendThrottle(throttle); // 0-2000
}
