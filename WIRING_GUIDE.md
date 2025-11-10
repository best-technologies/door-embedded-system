# 4.0" TFT 480x320 Display Wiring Guide

## ESP32 to TFT Display Connections

### Display Pins:
| TFT Pin | ESP32 Pin | Description |
|---------|-----------|-------------|
| VCC     | 3.3V      | Power supply |
| GND     | GND       | Ground |
| CS      | GPIO 5    | Chip Select |
| RESET   | GPIO 4    | Reset |
| DC/RS   | GPIO 2    | Data/Command |
| SDI/MOSI| GPIO 23   | SPI Data In |
| SCK     | GPIO 18   | SPI Clock |
| LED     | GPIO 15   | Backlight (optional) |
| SDO/MISO| GPIO 19   | SPI Data Out |

### Touch Controller Pins:
| Touch Pin | ESP32 Pin | Description |
|-----------|-----------|-------------|
| T_CLK     | GPIO 18   | Touch Clock (shared with display) |
| T_CS      | GPIO 21   | Touch Chip Select |
| T_DIN     | GPIO 23   | Touch Data In (shared with display) |
| T_DO      | GPIO 19   | Touch Data Out (shared with display) |
| T_IRQ     | GPIO 22   | Touch Interrupt (optional) |

## Library Compatibility Analysis

### ✅ **TFT_eSPI (RECOMMENDED)**
- **Best choice** for your 480x320 display
- Hardware optimized for ESP32
- Supports ILI9488 driver (common for 480x320 displays)
- Built-in touch support
- Excellent performance

### ⚠️ **Adafruit ILI9341**
- Limited to 320x240 resolution
- **NOT suitable** for 480x320 displays
- Good for smaller displays only

### ✅ **XPT2046_Touchscreen**
- Perfect for XPT2046 touch controllers
- Works with both TFT_eSPI and Adafruit libraries
- Reliable touch detection

## Display Controller Detection

Your 4.0" 480x320 display likely uses one of these controllers:
- **ILI9488** (most common for 480x320)
- **ST7796** (alternative)
- **ILI9486** (less common)

## Testing Steps

1. **Upload the main.cpp code**
2. **Check Serial Monitor** for initialization messages
3. **Verify display colors** (red, green, blue sequence)
4. **Test touch functionality** by touching the screen
5. **Adjust touch calibration** if coordinates seem off

## Troubleshooting

### Display Issues:
- **No display**: Check power and wiring
- **Wrong colors**: Try different driver (ILI9488 vs ST7796)
- **Garbled display**: Check SPI frequency (try 20MHz instead of 27MHz)

### Touch Issues:
- **No touch response**: Check T_CS and T_IRQ connections
- **Wrong coordinates**: Adjust mapping values in code
- **Inverted touch**: Change rotation settings

## Alternative Configurations

If the current setup doesn't work, try these driver alternatives in User_Setup.h:

```cpp
// Try ST7796 instead of ILI9488
#define ST7796_DRIVER

// Or try ILI9486
#define ILI9486_DRIVER
```