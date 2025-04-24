#ifndef DSHOT_X4_H
#define DSHOT_X4_H

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

	/**
	 * @brief checks if there was an error during initialisation of the DShot driver
	 *
	 * define DSHOT_DEBUG in src/dshot_config.h to enable information on Serial why the initialisation failed
	 *
	 * @return true if there was an error
	 * @return false if everything worked fine
	 */
	bool initError() {
		return iError;
	}

private:
	PIO pio; /// which PIO is used for the DShot driver
	uint8_t pinBase; /// the first pin that is used for DShot output/input
	uint8_t pinCount; /// the assigned pin count (up to 4)
	uint8_t sm; /// which state machine is used for the DShot driver
	uint32_t speed; /// speed in kBaud, e.g. 600 for DShot600
	uint8_t offset; /// program offset in the PIO instruction memory (needed to point to the same memory location in the next driver)
	bool iError = false; /// shows if there was an error during initialisation

	/**
	 * @brief appends a checksum to the outgoing DShot packet
	 *
	 * nibble-wise XOR, then bitwise invert.
	 *
	 * @param data 12 bit LSB-aligned (right-aligned) packet data (11 bits data + 1 bit telemetry)
	 * @return uint16_t 16 bit full packet with checksum appended
	 */
	static uint16_t appendChecksum(uint16_t data);
};

#endif // DSHOT_X4_H
