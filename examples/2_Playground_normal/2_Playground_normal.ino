/**
 * For more info on the library usage, see the docs or other easier examples.
 * Type a value between 0 and 2000 in the serial monitor to send a throttle value.
 */

#include <PIO_DShot.h>

#define PIN_BASE 10
#define PIN_COUNT 4

DShotX4 *esc;
uint16_t throttle = 0;

void setup() {
	Serial.begin(115200);
	esc = new DShotX4(PIN_BASE, PIN_COUNT);
}

void loop() {
	// sendThrottles is non-blocking. That means, we must not send it too fast (manual delayMicroseconds).
	// at DShot600, the maximum theoretical loop rate is about 35kHz, but it is unlikely that any ESC will use all those values, if it even works. Here we use roughly 5kHz.
	delayMicroseconds(200);

	uint16_t throttles[4] = {throttle, throttle, throttle, throttle}; // throttle values for each motor. Always hand over an array of 4 values, even if you only use one motor. The other values will be ignored.
	esc->sendThrottles(throttles);

	// serial stuff
	if (Serial.available()) {
		delay(3); // wait for the rest of the input
		String s = "";
		while (Serial.available()) {
			s += (char)Serial.read();
		}
		int32_t t = s.toInt();
		t = constrain(t, 0, 2000);
		throttle = t;
	}
}