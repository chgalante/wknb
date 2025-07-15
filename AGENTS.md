# AGENTS.md - Zephyr RTOS Project Guide

## Build Commands
- **Build**: `ninja -C build` or `cmake --build build`
- **Clean**: `ninja -C build clean` or `rm -rf build && cmake -B build`
- **Flash**: `./flash build/zephyr/zephyr.uf2` (custom script for RP2350)

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
  - Pin 16: Button (with internal pull-down)
  - Pin 25: LED (onboard, active high)
- **Internal Pull Resistors**: RP2350 has ~50-60kΩ internal pull-ups/downs
  - Use `GPIO_PULL_UP` for rotary encoder pins (normally high, grounded by encoder)
  - Use `GPIO_PULL_DOWN` for button pins (normally low, pulled high when pressed)
  - Configure in `gpio_dt_spec.dt_flags` field
- **USB HID**: Consumer Control interface for volume up/down commands

## Project Structure
- `app/src/main.c` - Main application code (USB HID rotary encoder)
- `app/prj.conf` - Zephyr configuration (USB, GPIO, logging)
- `app/CMakeLists.txt` - Build configuration
- `build/` - Build artifacts (ninja-based)
- `flash` - Custom flashing script for RP2350 bootloader

## Testing
- No automated tests configured - manual hardware testing required
- Use `printk()` statements for debugging GPIO and USB HID events