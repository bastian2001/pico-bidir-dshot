// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// -------------- //
// bidir_dshot_x1 //
// -------------- //

#define bidir_dshot_x1_wrap_target 0
#define bidir_dshot_x1_wrap 27

static const uint16_t bidir_dshot_x1_program_instructions[] = {
	//     .wrap_target
	0x8020, //  0: push   block
	0xe081, //  1: set    pindirs, 1
	0x80a0, //  2: pull   block
	0x6070, //  3: out    null, 16
	0xee00, //  4: set    pins, 0                [14]
	0x6e01, //  5: out    pins, 1                [14]
	0xe801, //  6: set    pins, 1                [8]
	0x00e4, //  7: jmp    !osre, 4
	0xe034, //  8: set    x, 20
	0xa0eb, //  9: mov    osr, !null
	0xe080, // 10: set    pindirs, 0
	0x01cb, // 11: jmp    pin, 11                [1]
	0xe046, // 12: set    y, 6
	0x000f, // 13: jmp    15
	0xe14d, // 14: set    y, 13                  [1]
	0x00d4, // 15: jmp    pin, 20
	0x008f, // 16: jmp    y--, 15
	0x4061, // 17: in     null, 1
	0x004e, // 18: jmp    x--, 14
	0x0000, // 19: jmp    0
	0xe146, // 20: set    y, 6                   [1]
	0x0017, // 21: jmp    23
	0xe14d, // 22: set    y, 13                  [1]
	0x00d9, // 23: jmp    pin, 25
	0x000c, // 24: jmp    12
	0x0097, // 25: jmp    y--, 23
	0x40e1, // 26: in     osr, 1
	0x0056, // 27: jmp    x--, 22
	//     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program bidir_dshot_x1_program = {
	.instructions = bidir_dshot_x1_program_instructions,
	.length = 28,
	.origin = -1,
};

static inline pio_sm_config bidir_dshot_x1_program_get_default_config(uint offset) {
	pio_sm_config c = pio_get_default_sm_config();
	sm_config_set_wrap(&c, offset + bidir_dshot_x1_wrap_target, offset + bidir_dshot_x1_wrap);
	return c;
}
#endif
