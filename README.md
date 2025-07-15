# wknb
wknb is a wired USB device that can be used to wake-up your PC, suspend it, 
change video outputs and control the volume remotely. This device was developed 
because I wanted a convenient way to play videogames from my couch, using my PC 
that is set-up in my office. The device is built with Zephyr RTOS and currently 
only converts rotary encoder input into volume control commands for your 
computer. The feature to wake-up, suspend and change video inputs are still a 
work in progress.

## Overview

This project implements a USB HID Consumer Control device using a Raspberry Pi 
Pico 2 (RP2350) and a rotary encoder. The device sends volume up/down commands 
to the host computer when the encoder is rotated, with configurable sensitivity 
to prevent accidental volume changes.

## Hardware

### Components
- **Microcontroller**: Raspberry Pi Pico 2 (RP2350)
- **Rotary Encoder**: Bourns PEC12R-4025F-S0024-ND (or compatible quadrature encoder)
- **Status LED**: Onboard LED (GPIO 25)

### Pin Configuration
| Component | GPIO Pin | Configuration |
|-----------|----------|---------------|
| Rotary A  | 14       | Input with internal pull-up |
| Rotary B  | 15       | Input with internal pull-up |
| LED       | 25       | Output (onboard LED) |

### Wiring
- Connect rotary encoder's A and B pins to GPIO 14 and 15 respectively
- Connect encoder's common pin to ground
- The RP2350's internal pull-up resistors (~50-60kΩ) are used, so no external resistors needed

## Features

- **USB HID Consumer Control**: Appears as a standard HID device to the host OS
- **Quadrature Decoding**: Proper rotary encoder state machine for reliable direction detection
- **Configurable Sensitivity**: Requires 6 detents before sending volume command (prevents accidental changes)
- **Visual Feedback**: LED blinks on startup and indicates rotation direction
- **Debounced Input**: 10ms polling interval with proper state transition handling

## Software Architecture

### Key Components
- **USB HID Stack**: Zephyr's USB HID class driver
- **GPIO Polling**: Direct GPIO polling with quadrature state machine
- **Consumer Control**: Standard HID usage codes for volume up/down (0xE9/0xEA)

### Configuration
- `NUMBER_OF_DETENTS_PER_REPORT`: Set to 6 (adjustable sensitivity)
- USB VID/PID: 0x1234/0x5678 (customize in `prj.conf`)
- Device name: "Rotary Encoder"

## Building and Flashing

### Prerequisites
- Zephyr SDK and toolchain
- CMake and Ninja build system
- West tool for Zephyr project management

### Build Commands
```bash
# Clean build
rm -rf build && cmake -B build

# Build the project
ninja -C build
# or
cmake --build build
```

### Flashing
```bash
# Flash to RP2350 (requires device in bootloader mode)
./flash build/zephyr/zephyr.uf2
```

The custom `flash` script handles the RP2350's UF2 bootloader protocol.

## Usage

1. Connect the device to your computer via USB
2. The device will appear as a HID Consumer Control device
3. Rotate the encoder clockwise to increase volume
4. Rotate the encoder counter-clockwise to decrease volume

### Startup Sequence
- Device blinks LED 3 times on startup to indicate successful initialization
- USB HID interface is registered and ready for use

## Customization

### Sensitivity Adjustment
Modify `NUMBER_OF_DETENTS_PER_REPORT` in `main.c` to change how many encoder detents are required before sending a volume command:
- Lower values = more sensitive (faster volume changes)
- Higher values = less sensitive (prevents accidental changes)

### USB Device Information
Edit `app/prj.conf` to customize:
- `CONFIG_USB_DEVICE_PRODUCT`: Device name
- `CONFIG_USB_DEVICE_MANUFACTURER`: Manufacturer name  
- `CONFIG_USB_DEVICE_VID/PID`: USB Vendor/Product IDs

### HID Commands
The device currently sends volume up/down commands. Modify the HID report descriptor and command codes in `main.c` to support other media keys or functions.

## Troubleshooting

### Device Not Recognized
- Ensure USB cable supports data transfer (not just power)
- Check that the device appears in system device manager/lsusb
- Verify USB HID drivers are installed

### Encoder Not Responding
- Check wiring connections to GPIO 14/15
- Verify encoder common pin is connected to ground
- Monitor debug output via UART console (GPIO 0/1)

### Sensitivity Issues
- Adjust `NUMBER_OF_DETENTS_PER_REPORT` value
- Check encoder mechanical specifications (detents per revolution)
- Verify quadrature signals with oscilloscope if available

## Development

### Code Style
Follows Zephyr RTOS conventions:
- Linux kernel style formatting
- Snake_case naming
- Zephyr-specific types and APIs

## License

See LICENSE file for details.
