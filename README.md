# Bidirectional DShot library for RP2040 / RP2350

[![GitHub Release](https://img.shields.io/github/v/release/bastian2001/pico-bidir-dshot?color=48c21a&label=Release)](https://github.com/bastian2001/pico-bidir-dshot/releases)
[![GitHub License](https://img.shields.io/github/license/bastian2001/pico-bidir-dshot?color=blue&label=License)](https://www.gnu.org/licenses/gpl-3.0)
[![All RPMs](https://img.shields.io/badge/RPMs-To_the_Moon-48c21a)](#)
[![Difficulty: Easy](https://img.shields.io/badge/Difficulty-Easy-blue)](#usage)
[![Pico SDK: C++](https://img.shields.io/badge/Pico--SDK-C++-48c21a)](#)

## Features

-   Easy to use
    -   No need for timers, interrupts or DMA
    -   Low setup and usage complexity
    -   ERPM packets are decoded in this library
-   Fast bidirectional communication
    -   Bidirectional and normal DShot up to 4800 (tested up to DShot 1200)
    -   Speed only limited by DShot protocol
    -   Fully asynchronous: no CPU intervention needed for sending or receiving
-   Oversampling with edge detection
    -   Telemetry unaffected by jitter/aliasing or clock differences between ESC and MCU
    -   Low CPU overhead: Edge detection is done on the PIO
-   Low usage of PIO hardware
    -   Bidirectional DShot needs 28 instructions and 1 state machine per ESC => max 8/12 ESCs
    -   Normal DShot needs 4 instructions and 1 state machine per 4 ESCs => max 30/48 ESCs
-   Extended DShot Telemetry support
    -   Read ESC temperature, voltage, current and more: all integrated
    -   See [here](https://github.com/bird-sanctuary/extended-dshot-telemetry) for more information

## Usage

This is essentially a bare minimum example for the Arduino IDE. For more details, check the integrated examples or the [Wiki](https://github.com/bastian2001/pico-bidir-dshot/wiki).

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
	rpm /= MOTOR_POLES / 2; // eRPM = RPM * poles/2 <=> RPM = eRPM / (poles/2)
	esc->sendThrottle(0); // 0-2000
}
```

Not calling `getTelemetryErpm` is fine if you don't care about that, but you definitely need to call `sendThrottle` regularly (recommended >500Hz), or else the ESC will time out because it thinks the main controller died.

## Installation

### Arduino IDE

1. Open the Arduino IDE and go to `Sketch` -> `Include Library` -> `Manage Libraries...`
2. In the Library Manager, search for `Pico_Bidir_DShot` and install it

### PlatformIO

Append this repo to your lib_deps in your `platformio.ini`:

```ini
lib_deps = https://github.com/bastian2001/pico-bidir-dshot.git
```

### CMake (bare Pico-SDK)

The code is written to use no Arduino.h for regular usage, so it can be used with the bare Pico SDK as well. Only the debug info uses Arduino's Serial class, so you can't enable those error hints without it. I personally never used CMake (manually), so sadly I can't help you with the installation.

## Roadmap

-   [x] Refactor code into library
-   [x] Add example code and test on RP2040
-   [x] Add documentation
-   [x] Adjust and test code for RP2350 (more PIOs)
-   [x] Release to Arduino Library Manager
-   [ ] Add more setups (e.g. DShotX1 - more efficient and versatile, DShotX8 - less PIOs needed)
-   [ ] Add more features (command queue, sendAll etc.)

## Contributing

If you feel like a feature is missing or something is broken, feel free to open an issue. I'm happy to help you with any questions you might have.

If you have the experience to fix the issue yourself, feel free to open a pull request. I'm happy to review and merge it.

## Credits

-   [DShot - The missing handbook](https://brushlesswhoop.com/dshot-and-bidirectional-dshot/): The only source I could find for bidirectional DShot
-   [Decoding a DShot eRPM packet](https://github.com/betaflight/betaflight/blob/master/src/main/drivers/dshot_bitbang_decode.c): Thanks for some sample code
-   [Extended DShot Telemetry](https://github.com/bird-sanctuary/extended-dshot-telemetry)
