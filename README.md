# wknb

wknb is a wired USB volume control device that lets you adjust your computer's 
volume remotely using a rotary encoder. Perfect for controlling audio from your 
couch while gaming or watching movies on a PC located in another room.

## Features

- **Volume Control**: Turn the rotary encoder to adjust your computer's volume up or down
- **USB Remote Wake-up**: Short button press wakes up your PC from sleep/suspend mode
- **System Suspend**: Long button press (2+ seconds) puts your computer to sleep
- **Smart Sensitivity**: Requires multiple encoder clicks before changing volume to prevent accidental adjustments
- **Visual Feedback**: LED provides status indication for startup, wake-up, and suspend operations
- **Plug & Play**: Works as a standard USB HID device - no drivers needed
- **Auto Stay-Awake**: Device automatically maintains USB activity to keep volume controls responsive

## Hardware Requirements

- **Microcontroller**: Raspberry Pi Pico 2 (RP2350)
- **Input**: Rotary encoder (Bourns PEC12R-4025F-S0024-ND or compatible)
- **Connection**: USB cable to your computer

## Pin Configuration

Connect the rotary encoder to the Raspberry Pi Pico 2 as follows:

| Component | Pico 2 Pin | GPIO | Configuration |
|-----------|------------|------|---------------|
| Rotary A  | Pin 14     | GP14 | Input with internal pull-up |
| Rotary B  | Pin 15     | GP15 | Input with internal pull-up |
| Button    | Pin 16     | GP16 | Input with internal pull-up (active-low) |
| LED       | Pin 25     | GP25 | Output (onboard LED) |

**Notes:**
- The rotary encoder pins (A/B) use internal pull-up resistors (~50-60kΩ) since they are normally high and grounded by the encoder
- The button pin uses internal pull-up resistor since it's normally high and grounded when pressed (active-low)
- The onboard LED (GP25) provides visual feedback for rotation, startup, and wake-up events

## Building and Flashing

### Prerequisites
- Zephyr RTOS development environment
- CMake and Ninja build tools
- West tool for Zephyr

### Build Instructions
```bash
# Activate the Python virtual environment
source ~/projects/wknb/.venv/bin/activate

# Debug build (with CDC ACM console for debugging)
west build -b rpi_pico2/rp2350a/m33 app --pristine

# Release build (HID only, no debug output)
west build -b rpi_pico2/rp2350a/m33 app --pristine -- -DCONF_FILE=prj_release.conf -DDTC_OVERLAY_FILE=app_release.overlay
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
- **Short button press** (< 2 seconds): Wakes up PC from sleep/suspend (when USB remote wake-up is enabled by the host)
- **Long button press** (≥ 2 seconds): Puts the computer to sleep/suspend
- **LED indicator**: 
  - Blinks 3 times on startup
  - Brief flash when wake-up is successful
  - Rapid 5 blinks when suspend command is sent
- **Auto-active**: Volume controls work immediately after plugging in (no button press required)

The device requires 6 encoder detents (clicks) before sending a volume command, preventing accidental volume changes from small movements.

### Wake-over-USB Setup

For the wake-up feature to work, your computer must support and enable USB remote wake-up:

**Windows**: 
- Device Manager → USB controllers → Your USB device → Properties → Power Management → "Allow this device to wake the computer"

**Linux**: 
- Check `/proc/acpi/wakeup` or use `echo enabled > /sys/bus/usb/devices/.../power/wakeup`

**macOS**: 
- System Preferences → Energy Saver → "Wake for network access" (may vary by version)

## License

See LICENSE file for details.
