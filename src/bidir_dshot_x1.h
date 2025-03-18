#include "dshot_global.h"
#include "hardware/pio.h"
#include <vector>
using std::vector;

enum class BidirDshotTelemetryType : uint8_t {
	ERPM,
	OTHER_VALUE,
	CHECKSUM_ERROR,
	NO_PACKET,
	VOLTAGE,
	CURRENT,
	TEMPERATURE,
	STATUS,
	STRESS,
	DEBUG_FRAME_1,
	DEBUG_FRAME_2,
};

class BidirDshotX1 {
public:
	static vector<BidirDshotX1 *> instances;

	/**
	 * @brief Initialize a new BidirDshotX1 instance
	 *
	 * @param pin the ESC pin
	 * @param speed DShot speed in kBaud, e.g. 600 for DShot600
	 * @param pio the PIO instance to use, default is pio0
	 * @param sm the state machine to use, default (-1) is autodetect
	 */
	BidirDshotX1(uint8_t pin, uint32_t speed = 600, PIO pio = pio0, int8_t sm = -1);

	/**
	 * @brief Deinitialize the BidirDshotX1 instance
	 *
	 * This will stop the state machine and free the pin. If this is the last instance on this PIO block, the PIO programm will be removed.
	 */
	~BidirDshotX1();

	/**
	 * @brief Send a throttle value to the ESC
	 *
	 * UART telemetry request bit IS NOT set (separate wire). Checksum is appended automatically.
	 *
	 * @param throttle the throttle value, 0-2000
	 */
	void sendThrottle(uint16_t throttle);

	/**
	 * @brief Send a raw packet to the ESC, useful for special commands
	 *
	 * UART telemetry request bit IS set (separate wire). See BidirDshotX1::CMD_ commands. Checksum is appended automatically.
	 *
	 * @param data the raw data to send, 11 bits, or 0-2047
	 */
	void sendRaw11Bit(uint16_t data);

	/**
	 * @brief Send a raw packet to the ESC, useful for special commands
	 *
	 * UART telemetry request bit can be set arbitrarily. Checksum is appended automatically.
	 *
	 * @param data the raw data to send, 12 bits: xxxx dddd dddd dddt where d is data, t is telemetry request bit and x is ignored
	 */
	void sendRaw12Bit(uint16_t data);

	/**
	 * @brief check if a telemetry packet is available
	 *
	 * Returns true regardless of the checksum validity or packet type.
	 *
	 * @return true if packet is available
	 * @return false if no packet is available
	 */
	bool checkTelemetryAvailable();

	/**
	 * @brief Get the current eRPM, provided the telemetry packet is valid and of type ERPM
	 *
	 * Leaves the erpm pointer unchanged if no packet is available, the checksum is invalid or the packet type is not ERPM. Safe to call even if no packet is available.
	 *
	 * @param erpm pointer to a uint32_t to store the erpm. Must be a valid pointer, not nullptr.
	 * @return BidirDshotTelemetryType ::ERPM, ::OTHER_VALUE, ::NO_PACKET or ::CHECKSUM_ERROR
	 */
	BidirDshotTelemetryType getTelemetryErpm(uint32_t *erpm);

	/**
	 * @brief Get the telemetry packet
	 *
	 * As long as a telemetry packet is available and valid, this function will overwrite the contents of the value pointer. Safe to call even if no packet is available. Not all types will be sent by all ESCs.
	 *
	 * If the packet is of type ERPM, the value will be the decoded eRPM. TEMPERATURE in °C. VOLTAGE in 250mV steps (e.g. 69 = 17.25 => divide by 4). CURRENT in 1A steps. DEBUG_FRAME_1 and DEBUG_FRAME_2 are the raw 8 bit values. STRESS from 0-255.
	 *
	 * STATUS frame: Bit[7] = alert event, Bit[6] = warning event, Bit[5] = error event, Bit[3-0] - Max. stress level [0-15]. Events may be defined differently by each ESC firmware.
	 *
	 * @param value pointer to a uint32_t to store the telemetry value. Must be a valid pointer, not nullptr.
	 * @return BidirDshotTelemetryType, all values except ::OTHER_VALUE may be returned
	 */
	BidirDshotTelemetryType getTelemetryPacket(uint32_t *value);

	/**
	 * @brief Get the raw telemetry packet
	 *
	 * As long as a telemetry packet is available and valid, this function will overwrite the contents of the value pointer. Safe to call even if no packet is available.
	 *
	 * Reads the raw telemetry packet from the ESC. 12 bits, e.g. in case of ERPM eeem mmmm mmmm (exponent, mantissa).
	 *
	 * @param value pointer to a uint32_t to store the telemetry value. Must be a valid pointer, not nullptr.
	 * @return BidirDshotTelemetryType, all values except ::OTHER_VALUE may be returned
	 */
	BidirDshotTelemetryType getTelemetryRaw(uint32_t *value);

	bool initError() {
		return iError;
	}

private:
	PIO pio;
	uint8_t pin;
	uint8_t sm;
	uint32_t speed;
	uint8_t offset;
	bool iError = false;

	static uint16_t appendChecksum(uint16_t data);
};