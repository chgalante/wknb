# wknb

wknb is a wired USB volume control device that lets you adjust your computer's 
volume remotely using a rotary encoder. Perfect for controlling audio from your 
couch while gaming or watching movies on a PC located in another room.

## What It Does

- **Volume Control**: Turn the rotary encoder to adjust your computer's volume up or down
- **Smart Sensitivity**: Requires multiple encoder clicks before changing volume to prevent accidental adjustments
- **Plug & Play**: Works as a standard USB HID device - no drivers needed

## Planned Features (Work in Progress)

- Wake up your PC remotely
- Suspend/sleep your computer
- Switch between video outputs

## Hardware Requirements

- **Microcontroller**: Raspberry Pi Pico 2 (RP2350)
- **Input**: Rotary encoder (Bourns PEC12R-4025F-S0024-ND or compatible)
- **Connection**: USB cable to your computer

## Pin Configuration

Connect the rotary encoder to the Raspberry Pi Pico 2 as follows:

| Component | Pico 2 Pin | GPIO | Configuration |
|-----------|------------|------|---------------|
| Rotary A  | Pin 14     | GP10 | Input with internal pull-up |
| Rotary B  | Pin 15     | GP11 | Input with internal pull-up |
| Button    | Pin 16     | GP12 | Input with internal pull-down |
| LED       | Pin 25     | GP25 | Output (onboard LED) |

**Notes:**
- The rotary encoder pins (A/B) use internal pull-up resistors (~50-60kΩ) since they are normally high and grounded by the encoder
- The button pin uses internal pull-down resistor since it's normally low and pulled high when pressed
- The onboard LED (GP25) provides visual feedback for rotation and startup

## Building and Flashing

### Prerequisites
- Zephyr RTOS development environment
- CMake and Ninja build tools
- West tool for Zephyr

### Build Instructions
```bash
# Clean build (if needed)
rm -rf build && cmake -B build

# Build the firmware
ninja -C build
# or alternatively: cmake --build build
```

### Flashing to Raspberry Pi Pico 2
1. **Enter bootloader mode**: Hold the BOOTSEL button while connecting the Pico 2 to your computer
2. **Flash the firmware**: Run the custom flash script
   ```bash
   ./flash build/zephyr/zephyr.uf2
   ```
3. **Verify**: The device should restart and the LED should blink on startup

The custom `flash` script handles copying the UF2 file to the RP2350 bootloader mass storage device.

## Setup

1. **Build the Hardware**: Connect the rotary encoder to the Raspberry Pi Pico 2 using the pin configuration above
2. **Flash the Firmware**: Follow the building and flashing instructions
3. **Connect**: Plug the device into your computer via USB
4. **Use**: Turn the encoder to control volume

The device appears as a standard HID input device and works with Windows, macOS, and Linux without additional drivers.

## Usage

- **Clockwise rotation**: Increases volume
- **Counter-clockwise rotation**: Decreases volume
- **LED indicator**: Blinks on startup and shows rotation feedback

The device requires 6 encoder detents (clicks) before sending a volume command, preventing accidental volume changes from small movements.

## License

See LICENSE file for details.
