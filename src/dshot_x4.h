#include "hardware/pio.h"
#include <vector>
using std::vector;

class DShotX4 {
public:
	static vector<DShotX4 *> instances;

	DShotX4() = delete;
	/**
	 * @brief Initialize a new DShotX4 instance
	 *
	 * @param pinBase the first ESC pin
	 * @param pinCount the number of ESC pins
	 * @param speed DShot speed in kBaud, e.g. 600 for DShot600
	 * @param pio the PIO instance to use, default is pio0
	 * @param sm the state machine to use, default (-1) is autodetect
	 */
	DShotX4(uint8_t pinBase, uint8_t pinCount, uint32_t speed = 600, PIO pio = pio0, int8_t sm = -1);

	/**
	 * @brief Deinitialize the DShotX4 instance
	 *
	 * This will stop the state machine and free the pin. If this is the last instance on this PIO block, the PIO programm will be removed.
	 */
	~DShotX4();

	/**
	 * @brief Send throttle values to the ESCs (array of 4)
	 *
	 * UART telemetry request bit IS NOT set (separate wire). Checksum is appended automatically. Call one of the send functions regularly (usually > 500Hz) to keep the ESC alive.
	 *
	 * @param throttle the throttle value, 0-2000
	 */
	void sendThrottles(uint16_t throttles[4]);

	/**
	 * @brief Send a raw packet to the ESCs, useful for special commands
	 *
	 * UART telemetry request bit IS set (separate wire). See DSHOT_CMD_ commands. Checksum is appended automatically. Call one of the send functions regularly (usually > 500Hz) to keep the ESC alive.
	 *
	 * @param data the raw data to send, 11 bits, or 0-2047
	 */
	void sendRaw11Bit(uint16_t data[4]);

	/**
	 * @brief Send a raw packet to the ESCs, useful for special commands
	 *
	 * UART telemetry request bit can be set arbitrarily. Checksum is appended automatically. Call one of the send functions regularly (usually > 500Hz) to keep the ESC alive.
	 *
	 * @param data the raw data to send, 12 bits: xxxx dddd dddd dddt where d is data, t is telemetry request bit and x is ignored
	 */
	void sendRaw12Bit(uint16_t data[4]);

	bool initError() {
		return iError;
	}

private:
	PIO pio;
	uint8_t pinBase;
	uint8_t pinCount;
	uint8_t sm;
	uint32_t speed;
	uint8_t offset;
	bool iError = false;

	static uint16_t appendChecksum(uint16_t data);
};