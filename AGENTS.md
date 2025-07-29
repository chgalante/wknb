# AGENTS.md - Zephyr RTOS Project Guide

## Build Commands
- **Setup**: `source ~/projects/wknb/.venv/bin/activate` (activate Python virtual environment)
- **Debug Build**: `west build -b rpi_pico2/rp2350a/m33 app --pristine` (HID + CDC ACM console)
- **Release Build**: `west build -b rpi_pico2/rp2350a/m33 app --pristine -- -DCONF_FILE=prj_release.conf -DDTC_OVERLAY_FILE=app_release.overlay` (HID only, no debug output)
- **Flash**: `./scripts/flash build/zephyr/zephyr.uf2` (custom script for RP2350)

## Code Style (follows Zephyr conventions)
- **Formatting**: Use `.clang-format` (Linux kernel style, 100 char limit, tabs for indentation)
- **Includes**: Local headers first `"file.h"`, then `<zephyr/*.h>`, then standard C headers
- **Naming**: snake_case for functions/variables, UPPER_CASE for macros/constants
- **Types**: Use Zephyr types (`uint8_t`, `int32_t`) and device tree specs (`gpio_dt_spec`)
- **Error handling**: Check return values, use `printk()` for debug output
- **GPIO**: Use device tree GPIO specs with `gpio_dt_spec` and `gpio_pin_*_dt()` functions
- **Memory**: Static allocation preferred, use Zephyr kernel APIs (`k_sleep`, `K_MSEC`)

## Hardware Configuration
- **Target**: Raspberry Pi Pico 2 (RP2350)
- **Rotary Encoder**: Bourns PEC12R-4025F-S0024-ND
  - Pin 14: Rotary A (with internal pull-up)
  - Pin 15: Rotary B (with internal pull-up) 
  - Pin 16: Button (with internal pull-up, active-low)
  - Pin 25: LED (onboard, active high)
- **Internal Pull Resistors**: RP2350 has ~50-60kΩ internal pull-ups/downs
  - Use `GPIO_PULL_UP` for rotary encoder pins (normally high, grounded by encoder)
  - Use `GPIO_PULL_UP` for button pin (normally high, grounded when pressed - active-low)
  - Configure in `gpio_dt_spec.dt_flags` field
- **USB HID**: Dual interface - Consumer Control (volume) + System Control (suspend)

## Project Structure
- `app/src/main.c` - Main application code (USB HID rotary encoder with wake-up and suspend)
- `app/prj.conf` - Debug configuration (HID + CDC ACM console)
- `app/prj_debug.conf` - Debug configuration (same as prj.conf)
- `app/prj_release.conf` - Release configuration (HID only, no debug)
- `app/app.overlay` - Debug device tree overlay (composite USB device)
- `app/app_release.overlay` - Release device tree overlay (HID only)
- `app/CMakeLists.txt` - Build configuration
- `build/` - Build artifacts (ninja-based)
- `scripts/flash` - Custom flashing script for RP2350 bootloader

## Testing
- No automated tests configured - manual hardware testing required
- **Debug Mode**: Use `DEBUG_PRINT()` statements for debugging GPIO and USB HID events
- **Debug Console**: Access via `cat /dev/ttyACM0` or `minicom -D /dev/ttyACM0 -b 115200`
- **Release Mode**: No debug output, `DEBUG_PRINT()` statements are compiled out

## Configuration Modes
- **Debug Mode** (default): Composite USB device with HID + CDC ACM console, debug output enabled
- **Release Mode**: HID-only USB device, all debug output disabled for production use

## Wake-over-USB and Suspend Implementation
- **Purpose**: Device can wake up suspended USB host and put active host to sleep
- **USB Remote Wakeup**: Enabled via `CONFIG_USB_DEVICE_REMOTE_WAKEUP=y`
- **Short Button Press** (< 2 seconds): Sends USB wake-up request when host is suspended
- **Long Button Press** (≥ 2 seconds): Sends System Sleep HID command to suspend active host
- **LED Feedback**: 
  - Blinks 3 times during bootup
  - Brief flash on successful wake-up
  - Rapid 5 blinks when suspend command is sent
- **Keep-alive Mechanism**: Sends null HID reports every 500ms to prevent USB suspension
- **USB State Management**: Tracks suspend/resume states via `usb_status_cb()` callback
- **Long-press Detection**: Uses Zephyr work queue with 2-second timer for button hold detection

## Key Functions
- `send_usb_keepalive()` - Sends null HID report to maintain USB activity
- `button_interrupt_handler()` - GPIO interrupt handler for button press/release events
- `long_press_work_handler()` - Work queue handler for long-press suspend detection
- `send_suspend_command()` - Sends System Sleep HID command with visual feedback
- `usb_status_cb()` - USB device status callback for suspend/resume events
- **Volume Controls**: Rotary encoder sends Consumer Control HID reports (Volume Up/Down)
- **Quadrature Decoding**: Uses state transition table for reliable encoder reading
- **Detent Counting**: Requires 6 detents per volume command to prevent accidental triggers
- **Button State Management**: Tracks press/release timing with `GPIO_INT_EDGE_BOTH`

## USB Enumeration Best Practices
- **Conservative Approach**: No wake-up requests during enumeration to avoid recognition issues
- **Delayed Initialization**: 100ms startup delay for power stabilization
- **Proper State Reset**: Handle USB_DC_RESET and USB_DC_DISCONNECTED events
- **Debug Logging**: Enhanced USB state logging for troubleshooting enumeration issues

## HID Report Structure
- **Consumer Control** (Report ID 1): 16-bit usage codes for Volume Up (0x00E9) and Volume Down (0x00EA)
- **System Control** (Report ID 2): 1-bit System Sleep command (0x82) with 7-bit padding
- **Dual Interface**: Device presents both Consumer and System Control collections in single descriptor
- **Report Format**: All reports include report ID as first byte, followed by usage-specific data

## Implementation Notes for Future Development
- **Timer Management**: Long-press detection uses `k_work_delayable` with `LONG_PRESS_DURATION_MS` (2000ms)
- **GPIO Configuration**: Button uses `GPIO_INT_EDGE_BOTH` to detect press and release events
- **State Variables**: `button_pressed_flag`, `button_press_time` track button state and timing
- **Work Queue**: System work queue handles long-press detection to avoid blocking interrupt context
- **Visual Feedback**: LED patterns differentiate between wake-up (brief flash) and suspend (5 rapid blinks)
- **Error Handling**: All HID transmissions check USB suspended state before sending commands