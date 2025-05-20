#include "dshot_x4.h"
#include "dshot_common.h"
#include "hardware/clocks.h"
#include "pio/dshotx4.pio.h"

vector<DShotX4 *> DShotX4::instances;

DShotX4::DShotX4(uint8_t pinBase, uint8_t pinCount, uint32_t speed, PIO pio, int8_t sm) {
#if DBG
	char pioStr[32] = "";
	pioToPioStr(pio, pioStr);
#else
	char *pioStr = nullptr;
#endif

	// ensure valid parameters
	if (sm >= 4 || sm < -1 || pinBase >= NUM_BANK0_GPIOS || pinCount > 4 || !pinCount || speed < 150 || speed > 4800 || (pio != pio0 && pio != pio1
		#if NUM_PIOS > 2
		&& pio != pio2
		#endif	
		)) {
		DEBUG_PRINTF("Invalid parameters: Check that sm is -1...3, pinBase is 0...29 (or 0...47 on RP2350), pinCount is 1...4, speed is 150...4800 and pio is pio0 or pio1 (or pio2 on RP2350). You supplied: sm=%d, pinBase=%d, pinCount=%d, speed=%d, pio=%s\n", sm, pinBase, pinCount, speed, pioStr);
		iError = true;
		return;
	}

	if (speed != 150 && speed != 300 && speed != 600 && speed != 1200 && speed != 2400) {
		DEBUG_PRINTF("Unofficial speed: %d. Unless you know what you are doing, please select DShot 150, 300, 600, 1200 or 2400.\n", speed);
	}

	// Check if SM is claimed, then claim it
	if (sm == -1) {
		sm = pio_claim_unused_sm(pio, false);
		if (sm < 0) {
			DEBUG_PRINTF("No free state machines available, pio=%s\n", pioStr);
			iError = true;
			return;
		}
	} else {
		if (pio_sm_is_claimed(pio, sm)) {
			DEBUG_PRINTF("SM provided but already claimed, pio=%s, sm=%d", pioStr, sm);
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
		if (pio_can_add_program(pio, &dshotx4_program)) {
			this->offset = pio_add_program(pio, &dshotx4_program);
		} else {
			DEBUG_PRINTF("No space for program on %s", pioStr);
			iError = true;
			pio_sm_unclaim(pio, sm);
			return;
		}
	} else {
		this->offset = o;
	}

	// set up GPIOs
	for (int i = 0; i < pinCount; i++) {
		uint8_t pin = pinBase + i;
		pio_gpio_init(pio, pin);
		gpio_set_pulls(pin, false, false);
	}

	// set up the state machine
	pio_sm_config c = dshotx4_program_get_default_config(this->offset);
	sm_config_set_set_pins(&c, pinBase, pinCount);
	sm_config_set_out_pins(&c, pinBase, pinCount);
	sm_config_set_out_shift(&c, false, false, 32);
	pio_sm_set_consecutive_pindirs(pio, this->sm, pinBase, pinCount, true);
	pio_sm_init(pio, this->sm, this->offset, &c);
	pio_sm_set_enabled(pio, this->sm, true);
	uint32_t targetClock = 12000000 / 300 * speed; // 12 MHz for DShot300
	uint32_t cpuClock = clock_get_hz(clk_sys);
	pio_sm_set_clkdiv(pio, this->sm, (float)cpuClock / targetClock);

	// add this instance to the list of instances
	this->pio = pio;
	this->pinBase = pinBase;
	this->pinCount = pinCount;
	this->speed = speed;
	this->iError = false;
	DShotX4::instances.push_back(this);
}

DShotX4::~DShotX4() {
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
	for (auto inst : DShotX4::instances) {
		if (inst != this && inst->pio == this->pio && inst->initError() == false) {
			isLast = false;
			break;
		}
	}
	if (isLast) {
		pio_remove_program(this->pio, &dshotx4_program, this->offset);
	}

	// free the pins
	for (int i = 0; i < this->pinCount; i++) {
		gpio_init(this->pinBase + i);
	}

	// remove this instance from the list of instances
	auto it = DShotX4::instances.begin();
	while (it != DShotX4::instances.end()) {
		if (*it == this) {
			it = DShotX4::instances.erase(it);
		} else {
			it++;
		}
	}
}

void DShotX4::sendThrottles(uint16_t throttles[4]) {
	// check if the throttle value is valid
	for (int i = 0; i < 4; i++) {
		if (throttles[i] > 2000) {
			throttles[i] = 2000;
		}
		if (throttles[i]) throttles[i] += 47;
		throttles[i] <<= 1;
	}
	this->sendRaw12Bit(throttles);
}

void DShotX4::sendRaw11Bit(uint16_t data[4]) {
	for (int i = 0; i < 4; i++)
		data[i] = (data[i] << 1) | 1;
	this->sendRaw12Bit(data);
}

void DShotX4::sendRaw12Bit(uint16_t data[4]) {
	for (int i = 0; i < 4; i++)
		data[i] = this->appendChecksum(data[i]);

	uint32_t motorPacket[2] = {0, 0};
	for (int i = 31; i >= 0; i--) {
		int pos = i / 4;
		int motor = i % 4;
		motorPacket[0] |= ((data[motor] >> (pos + 8)) & 1) << i;
		motorPacket[1] |= ((data[motor] >> pos) & 1) << i;
	}
	pio_sm_put(this->pio, this->sm, motorPacket[0]);
	pio_sm_put(this->pio, this->sm, motorPacket[1]);
}

uint16_t DShotX4::appendChecksum(uint16_t data) {
	int csum = data;
	csum ^= data >> 4;
	csum ^= data >> 8;
	csum = csum;
	csum &= 0xF;
	return (data << 4) | csum;
}