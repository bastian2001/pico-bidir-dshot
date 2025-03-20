/**
 * For more info on the library usage, see the docs or other easier examples.
 * It is expected, that you have done all the examples before this one.
 *
 * This is the most advanced example. It uses Extended DShot telemetry to not just get the RPM, but also all the other telemetry values. It is controlled via commands:
 * - Send your Serial Monitor or Serial Plotter to New line mode (\n)
 * - Type T1000 for sending a throttle value of 1000 (0-2000)
 * - Type C3 for sending a special command (0-47) -> 3 = beacon 3 (only send them when stopped, and all commands are sent 10 times in this example)
 * - Type E to enable Extended DShot telemetry (this is the same as sending C13)
 *
 * The Serial Monitor will print the throttle value and all other available telemetry values. Not all ESCs support all telemetry values.
 */

#include <PIO_DShot.h>

#define PIN 10
#define MOTOR_POLES 14

BidirDShotX1 *esc;
uint16_t throttle = 0;
uint32_t rpm = 0;
uint32_t temp = 0;
float voltage = 0;
uint32_t current = 0;
uint32_t lastStatus = 0;
uint32_t stress = 0;

void setup() {
	Serial.begin(115200);
	delay(7000); // wait for serial monitor to open
	Serial.println("Thrott\tRPM\tVoltage\tAmps\tTemp °C\tStress\tStatus");
	esc = new BidirDShotX1(PIN);
}

void loop() {
	delayMicroseconds(200);

	uint32_t returnValue = 0;
	BidirDshotTelemetryType type = esc->getTelemetryPacket(&returnValue);
	switch (type) {
	case BidirDshotTelemetryType::ERPM:
		rpm = returnValue / (MOTOR_POLES / 2);
		break;
	case BidirDshotTelemetryType::VOLTAGE:
		voltage = (float)returnValue / 4;
		break;
	case BidirDshotTelemetryType::CURRENT:
		current = returnValue;
		break;
	case BidirDshotTelemetryType::TEMPERATURE:
		temp = returnValue;
		break;
	case BidirDshotTelemetryType::STATUS:
		lastStatus = returnValue;
		break;
	case BidirDshotTelemetryType::STRESS:
		stress = returnValue & ESC_STATUS_MAX_STRESS_MASK;
		break;

	// other possible cases are:
	case BidirDshotTelemetryType::DEBUG_FRAME_1:
		// custom ESC telemetry, not used in regular ESCs
	case BidirDshotTelemetryType::DEBUG_FRAME_2:
	// custom ESC telemetry, not used in regular ESCs
	case BidirDshotTelemetryType::CHECKSUM_ERROR:
	// Means the last packet was received, but corrupted. This is not a problem, just ignore it.
	case BidirDshotTelemetryType::NO_PACKET:
	// No telemetry packet available. Either the wait time between writing the last DShot packet and reading the telemetry packet was too short, or the ESC is not powered. This is not a problem, just ignore it.
	default:
		break;
	}

	// serial stuff
	static uint32_t lastTime = 0;
	if (millis() - lastTime > 100) {
		lastTime = millis();
		Serial.print(throttle);
		Serial.print("\t");
		Serial.print(rpm);
		Serial.print("\t");
		Serial.print(voltage);
		Serial.print("\t");
		Serial.print(current);
		Serial.print("\t");
		Serial.print(temp);
		Serial.print("\t");
		Serial.print(stress);
		Serial.print("\t");
		Serial.println(lastStatus, BIN);
	}

	if (Serial.available()) {
		delay(3); // wait for the rest of the input
		char cmd = Serial.read();
		String s = "";
		while (Serial.available()) {
			s += (char)Serial.read();
		}
		uint32_t value = s.toInt();
		switch (cmd) {
		case 'T':
			throttle = value;
			break;
		case 'C':
			sendSpecialCommand(value);
			break;
		case 'E':
			sendSpecialCommand(13);
			break;
		default:
			throttle = 0;
			break;
		}
	}

	esc->sendThrottle(throttle);
}

void sendSpecialCommand(uint16_t cmd) {
	for (int i = 0; i < 10; i++) {
		esc->sendRaw11Bit(cmd);
		delayMicroseconds(200);
	}
}
