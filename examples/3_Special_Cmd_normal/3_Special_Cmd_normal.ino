/**
 * For more info on the library usage, see the docs or other easier examples.
 * Type a value between 0 and 2047 in the serial monitor to send a value. 48-2047 are throttle values, 0-47 are special commands.
 * The special commands are documented here: https://brushlesswhoop.com/dshot-and-bidirectional-dshot/
 * They are also available with these names:
 * DSHOT_CMD_MOTOR_STOP = 0
 * DSHOT_CMD_BEACON1
 * DSHOT_CMD_BEACON2
 * DSHOT_CMD_BEACON3
 * DSHOT_CMD_BEACON4
 * DSHOT_CMD_BEACON5
 * DSHOT_CMD_ESC_INFO
 * DSHOT_CMD_SPIN_DIRECTION_1
 * DSHOT_CMD_SPIN_DIRECTION_2
 * DSHOT_CMD_3D_MODE_OFF
 * DSHOT_CMD_3D_MODE_ON
 * DSHOT_CMD_SETTINGS_REQUEST
 * DSHOT_CMD_SAVE_SETTINGS
 * DSHOT_CMD_EXTENDED_TELEMETRY_ENABLE
 * DSHOT_CMD_EXTENDED_TELEMETRY_DISABLE = 14
 * DSHOT_CMD_SPIN_DIRECTION_NORMAL = 20
 * DSHOT_CMD_SPIN_DIRECTION_REVERSED
 * DSHOT_CMD_LED0_ON
 * DSHOT_CMD_LED1_ON
 * DSHOT_CMD_LED2_ON
 * DSHOT_CMD_LED3_ON
 * DSHOT_CMD_LED0_OFF
 * DSHOT_CMD_LED1_OFF
 * DSHOT_CMD_LED2_OFF
 * DSHOT_CMD_LED3_OFF
 * DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF
 * DSHOT_CMD_SILENT_MODE_ON_OFF = 31
 */

#include <PIO_DShot.h>

#define PIN_BASE 10
#define PIN_COUNT 4

DShotX4 *esc;
uint16_t value = 0;

void setup() {
	Serial.begin(115200);
	esc = new DShotX4(PIN_BASE, PIN_COUNT);
}

void loop() {
	// sendRaw11Bit is non-blocking. That means, we must not send it too fast (manual delayMicroseconds).
	// at DShot600, the maximum theoretical loop rate is about 35kHz, but it is unlikely that any ESC will use all those values, if it even works. Here we use roughly 5kHz.
	delayMicroseconds(200);

	uint16_t val[4] = {value, value, value, value}; // throttle values for each motor. Always hand over an array of 4 values, even if you only use one motor. The other values will be ignored.
	esc->sendRaw11Bit(val);

	// esc->sendRaw12Bit is also available, if you want to set the telemetry request bit manually (i.e. send a custom packet)
	// The bit sequence is: vvvv vvvv vvvt, where t is the telemetry request bit. The rest is the value.

	// serial stuff
	if (Serial.available()) {
		delay(3); // wait for the rest of the input
		String s = "";
		while (Serial.available()) {
			s += (char)Serial.read();
		}
		value = s.toInt();
		value = constrain(value, 0, 2047);
	}
}
