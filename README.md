# Bidirectional DShot library for RP2040 / RP2350

> [!NOTE]
> This library is still in development and not yet ready for use. Please check in a few days. While the code itself is tested on my FC, refactoring for the library is still ongoing.

## Roadmap

-   [x] Refactor code into library
-   [x] Add example code and test on RP2040
-   [x] Add documentation
-   [x] Adjust code for RP2350 (more PIOs)
-   [ ] Test on RP2350
-   [x] Release to Arduino Library Manager
-   [ ] Add more setups (e.g. DShotX1 - more efficient and versatile, DShotX8 - less PIO usage)
-   [ ] Add more features (command queue, sendAll etc.)

## Features

-   Easy to use
    -   no need for timer, interrupts or DMA
    -   low setup and usage complexity
    -   eRPMs are calculated in this library
-   Fast bidirectional communication
    -   Bidirectional and normal DShot up to 4800 (tested up to 1200)
    -   Speed only limited by DShot protocol
    -   Fully asynchronous: no CPU intervention needed for sending or receiving
-   14x Oversampling with edge detection
    -   unaffected by jitter/aliasing or clock differences between ESC and MCU
    -   Low CPU overhead: Edge detection is done on the PIO
-   Low usage of PIO hardware
    -   bidirectional DShot needs 28 instructions and 1 state machine = 1 ESC => max 8/12 ESCs
    -   normal DShot only needs 4 instructions and 1 state machine = 4 ESCs => max 30/48 ESCs
-   Extended DShot Telemetry support
    -   Read ESC temperature, voltage, current and more: all integrated
    -   See [here](https://github.com/bird-sanctuary/extended-dshot-telemetry) for more information
-   Reliable and battle-tested in my [flight controller](https://github.com/bastian2001/Kolibri-FC)

## Installation

### PlatformIO

1. Requires Earle F. Philhower, III's arduino-pico port [Installation](https://arduino-pico.readthedocs.io/en/latest/install.html)
2. Append this repo to your lib_deps in your `platformio.ini`:

```ini
lib_deps = https://github.com/bastian2001/pico-bidir-dshot.git
```

### Arduino IDE

1. Requires Earle F. Philhower, III's arduino-pico port [Installation](https://arduino-pico.readthedocs.io/en/latest/install.html)
2. Open the Arduino IDE and go to `Sketch` -> `Include Library` -> `Manage Libraries...`
3. In the Library Manager, search for `DShot_for_RP2040_RP2350_Pi_Pico` and install it

## Usage

This is essentially a bare minimum example. For more details, check the integrated examples or the [Wiki](https://github.com/bastian2001/pico-bidir-dshot/wiki).

Not calling `getTelemetryErpm` is fine if you don't care about that, but you definitely need to call `sendThrottle` regularly (recommended >500Hz), or else the ESC will time out because it thinks the main controller died.

```cpp
#include <PIO_DShot.h>
#define MOTOR_POLES 14

BidirDShotX1 *esc;

void setup() {
	esc = new BidirDShotX1(10, 600); // pin 10, DShot600
}

void loop() {
	delayMicroseconds(200); // keep packets spaced out
	uint32_t rpm = 0;
	esc->getTelemetryErpm(&rpm);
	rpm /= MOTOR_POLES / 2; // eRPM = RPM * poles/2
	esc->sendThrottle(0); // 0-2000
}
```

## Contributing

If you feel like a feature is missing or something is broken, feel free to open an issue. I'm happy to help you with any questions you might have.

If you have the experience to fix the issue yourself, feel free to open a pull request. I'm happy to review and merge it.

## Credits

-   [DShot - The missing handbook](https://brushlesswhoop.com/dshot-and-bidirectional-dshot/): The only source I could find for bidirectional DShot
-   [Decoding a DShot eRPM packet](https://github.com/betaflight/betaflight/blob/master/src/main/drivers/dshot_bitbang_decode.c): Thanks for some sample code
-   [Extended DShot Telemetry](https://github.com/bird-sanctuary/extended-dshot-telemetry)
