// TFT_eSPI User Setup for 4.0" 480x320 TFT Display with ESP32
#define USER_SETUP_INFO "ESP32_480x320_TFT"

// Driver selection - Use ILI9488 for 480x320 displays
#define ILI9488_DRIVER

// ESP32 Pin definitions for 4.0" TFT 480x320 display
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5   // Chip select control pin
#define TFT_DC   2   // Data Command control pin
#define TFT_RST  4   // Reset pin
// #define TFT_BL   15  // LED back-light control pin (optional)

// Touch screen chip select pin
#define TOUCH_CS 21

// SPI frequency settings
#define SPI_FREQUENCY  27000000  // 27MHz for display
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000  // 2.5MHz for touch

// Font loading
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT