/**
 * For more info on the library usage, see the docs or other easier examples.
 * Type a value between 0 and 2000 in the serial monitor to send a throttle value.
 * It will send the RPM and current throttle to the Serial monitor every 100ms.
 */

#include <PIO_DShot.h>

#define PIN 10
#define MOTOR_POLES 14

BidirDShotX1 *esc;
volatile uint16_t throttle = 0;
volatile uint32_t rpm = 0;

void setup() {
	Serial.begin(115200);
	esc = new BidirDShotX1(PIN);
}

void loop() {
	// we need at least 30ish us + two packet lengths (53us at DShot600) before we can retrieve the rpm (total 83ish us).
	// all of this happens asynchronously. getTelemetryErpm() will not block. If there's no telemetry available, it will just do nothing.
	// we also need at least 60ish us + 2 packet lengths (53us at DShot600 => 113ish us) before we can send a new throttle value.
	// Check the docs if you want to know more about the timing. It is possible to just send the throttle value directly after getting the rpm though, as long as there is enough time between the two sent frames.
	delayMicroseconds(200);
	uint32_t r = 0;
	esc->getTelemetryErpm(&r);
	r /= MOTOR_POLES / 2; // eRPM = RPM * poles/2
	rpm = r;
	esc->sendThrottle(throttle);

	// serial stuff
	static uint32_t lastTime = 0;
	if (millis() - lastTime > 100) {
		lastTime = millis();
		Serial.print(throttle);
		Serial.print("\t");
		Serial.println(rpm);
	}

	if (Serial.available()) {
		delay(3); // wait for the rest of the input
		String s = "";
		while (Serial.available()) {
			s += (char)Serial.read();
		}
		uint32_t t = s.toInt();
		t = constrain(t, 0, 2000);
		throttle = t;
	}
}