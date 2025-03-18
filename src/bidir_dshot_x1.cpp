#include "bidir_dshot_x1.h"
#include "hardware/clocks.h"
#include "pio/bidir_dshot_x1.pio.h"

vector<BidirDshotX1 *> BidirDshotX1::instances;

#define iv 0xFFFFFFFF
const uint32_t escDecodeLut[32] = {
	iv, iv, iv, iv, iv, iv, iv, iv, iv, 9, 10, 11, iv, 13, 14, 15,
	iv, iv, 2, 3, iv, 5, 6, 7, iv, 0, 8, 1, iv, 4, 12, iv};

const BidirDshotTelemetryType telemetryTypeLut[16] = {BidirDshotTelemetryType::ERPM, BidirDshotTelemetryType::ERPM, BidirDshotTelemetryType::TEMPERATURE, BidirDshotTelemetryType::ERPM, BidirDshotTelemetryType::VOLTAGE, BidirDshotTelemetryType::ERPM, BidirDshotTelemetryType::CURRENT, BidirDshotTelemetryType::ERPM, BidirDshotTelemetryType::DEBUG_FRAME_1, BidirDshotTelemetryType::ERPM, BidirDshotTelemetryType::DEBUG_FRAME_2, BidirDshotTelemetryType::ERPM, BidirDshotTelemetryType::STRESS, BidirDshotTelemetryType::ERPM, BidirDshotTelemetryType::STATUS, BidirDshotTelemetryType::ERPM};

BidirDshotX1::BidirDshotX1(uint8_t pin, uint32_t speed, PIO pio, int8_t sm) {
	// ensure valid parameters
	if (sm >= 4 || sm < -1 || pin > 29 || speed < 150 || speed > 4800 || (pio != pio0 && pio != pio1)) {
		// Bidir Dshot 150 is not supported, but since the protocol itself is fine with it, it is allowed here
		iError = true;
		return;
	}

	// Check if SM is claimed, then claim it
	if (sm == -1) {
		sm = pio_claim_unused_sm(pio, false);
		if (sm < 0) {
			iError = true;
			return;
		}
	} else {
		if (pio_sm_is_claimed(pio, sm)) {
			iError = true;
			return;
		}
		pio_sm_claim(pio, sm);
	}
	this->sm = sm;

	// check if the program is already loaded, if not, try to load it
	uint8_t o = 255;
	for (auto inst : this->instances) {
		if (inst->pio == pio && inst->initError() == false) {
			o = inst->offset;
			break;
		}
	}
	if (o == 255) {
		if (pio_can_add_program(pio, &bidir_dshot_x1_program)) {
			this->offset = pio_add_program(pio, &bidir_dshot_x1_program);
		} else {
			iError = true;
			pio_sm_unclaim(pio, sm);
			return;
		}
	} else {
		this->offset = o;
	}

	// set up GPIO
	pio_gpio_init(pio, pin);
	gpio_set_pulls(pin, true, false);

	// set up the state machine
	pio_sm_config c = bidir_dshot_x1_program_get_default_config(this->offset);
	sm_config_set_set_pins(&c, pin, 1);
	sm_config_set_out_pins(&c, pin, 1);
	sm_config_set_in_pins(&c, pin);
	sm_config_set_jmp_pin(&c, pin);
	sm_config_set_out_shift(&c, false, false, 32);
	sm_config_set_in_shift(&c, false, false, 32);
	pio_sm_init(pio, this->sm, this->offset, &c);
	pio_sm_set_consecutive_pindirs(pio, this->sm, pin, 1, true);
	pio_sm_set_enabled(pio, this->sm, true);
	uint32_t targetClock = 12000000 * speed / 300; // 12 MHz for DShot300
	uint32_t cpuClock = clock_get_hz(clk_sys);
	pio_sm_set_clkdiv(pio, this->sm, (float)cpuClock / targetClock);

	// add this instance to the list of instances
	this->pio = pio;
	this->pin = pin;
	this->speed = speed;
	this->iError = false;
	BidirDshotX1::instances.push_back(this);
}

BidirDshotX1::~BidirDshotX1() {
	// if this instance is not initialized, do nothing
	if (this->iError) {
		return;
	}

	// stop the state machine
	pio_sm_set_enabled(this->pio, this->sm, false);
	if (this->sm >= 0) {
		pio_sm_unclaim(this->pio, this->sm);
	}
	bool isLast = true;
	for (auto inst : BidirDshotX1::instances) {
		if (inst != this && inst->pio == this->pio && inst->initError() == false) {
			isLast = false;
			break;
		}
	}
	if (isLast) {
		pio_remove_program(this->pio, &bidir_dshot_x1_program, this->offset);
	}

	// free the GPIO pin => pull up to reduce artifacts
	gpio_set_pulls(this->pin, true, false);
	gpio_set_dir(this->pin, GPIO_IN);
	gpio_set_function(this->pin, GPIO_FUNC_NULL);

	// remove this instance from the list of instances
	auto it = BidirDshotX1::instances.begin();
	while (it != BidirDshotX1::instances.end()) {
		if (*it == this) {
			it = BidirDshotX1::instances.erase(it);
		} else {
			it++;
		}
	}
}

void BidirDshotX1::sendThrottle(uint16_t throttle) {
	// check if the throttle value is valid
	if (throttle > 2000) {
		throttle = 2000;
	}

	if (throttle) throttle += 47;
	throttle <<= 1;
	this->sendRaw12Bit(throttle);
}

void BidirDshotX1::sendRaw11Bit(uint16_t data) {
	data = (data << 1) | 1;
	this->sendRaw12Bit(data);
}

void BidirDshotX1::sendRaw12Bit(uint16_t data) {
	data = this->appendChecksum(data);

	if (pio_sm_get_pc(this->pio, this->sm) != this->offset + 2)
		pio_sm_exec(pio, sm, pio_encode_jmp(this->offset + 1));
	pio_sm_put(this->pio, this->sm, data);
}

uint16_t BidirDshotX1::appendChecksum(uint16_t data) {
	int csum = data;
	csum ^= data >> 4;
	csum ^= data >> 8;
	csum = ~csum;
	csum &= 0xF;
	return (data << 4) | csum;
}

bool BidirDshotX1::checkTelemetryAvailable() {
	return !pio_sm_is_rx_fifo_empty(this->pio, this->sm);
}

BidirDshotTelemetryType BidirDshotX1::getTelemetryErpm(uint32_t *value) {
	uint32_t raw;
	BidirDshotTelemetryType ret = this->getTelemetryRaw(&raw);
	if (ret > BidirDshotTelemetryType::NO_PACKET) {
		return BidirDshotTelemetryType::OTHER_VALUE;
	}
	if (ret > BidirDshotTelemetryType::ERPM) {
		return ret;
	}
	raw = (raw & 0x1FF) << (raw >> 9); // eeem mmmm mmmm
	if (!raw) {
		return BidirDshotTelemetryType::CHECKSUM_ERROR; // not quite right, but close enough
	}
	raw = (60000000 + 50 * raw) / raw;
	*value = raw;
	return BidirDshotTelemetryType::ERPM;
}

BidirDshotTelemetryType BidirDshotX1::getTelemetryPacket(uint32_t *value) {
	uint32_t raw;
	BidirDshotTelemetryType ret = this->getTelemetryRaw(&raw);

	if (ret == BidirDshotTelemetryType::ERPM) {
		raw = (raw & 0x1FF) << (raw >> 9); // eeem mmmm mmmm
		if (!raw) {
			return BidirDshotTelemetryType::CHECKSUM_ERROR; // not quite right, but close enough
		}
		raw = (60000000 + 50 * raw) / raw;
		*value = raw;
	} else if (ret > BidirDshotTelemetryType::NO_PACKET) {
		*value = raw & 0xFF;
	}
	return ret;
}

BidirDshotTelemetryType BidirDshotX1::getTelemetryRaw(uint32_t *value) {
	if (pio_sm_is_rx_fifo_empty(this->pio, this->sm)) {
		return BidirDshotTelemetryType::NO_PACKET;
	}

	// get most current data
	uint32_t raw = pio_sm_get_blocking(this->pio, this->sm);
	while (!pio_sm_is_rx_fifo_empty(this->pio, this->sm)) {
		raw = pio_sm_get_blocking(this->pio, this->sm);
	}

	raw = raw ^ (raw >> 1);
	uint32_t data = escDecodeLut[raw & 0x1F];
	data |= escDecodeLut[(raw >> 5) & 0x1F] << 4;
	data |= escDecodeLut[(raw >> 10) & 0x1F] << 8;
	data |= escDecodeLut[(raw >> 15) & 0x1F] << 12;
	uint32_t checksum = (data >> 8) ^ data;
	checksum ^= checksum >> 4;
	if (checksum != 0x0F || data > 0xFFFF) {
		return BidirDshotTelemetryType::CHECKSUM_ERROR;
	}

	*value = data >> 4;
	return telemetryTypeLut[data >> 12];
}