# PicoW Weather Station

A self-powered, e-paper weather station built with the **Raspberry Pi Pico W**, displaying live weather conditions from **wttr.in** on a **Waveshare 2.9" Tri-color E-Paper Display**.

The device connects to Wi-Fi, fetches ASCII art weather data, renders it on the screen, and then enters a deep sleep state for 1 hour to conserve power.

## Features

- **Wi-Fi Connectivity:** Connects to your local network using pico_cyw43_arch.
- **Live Weather Data:** Fetches real-time conditions (temperature, wind, precipitation) from wttr.in.
- **ASCII Art Rendering:** Displays beautiful terminal-style weather graphics on the e-paper display.
- **Power Efficient:** Idles 1 hour between updates; the e-paper display retains the image with zero power draw while the device waits.
- **Modular Design:** Layered architecture — main.c orchestrates independent wifi, weather, and display modules over a shared network abstraction (lwIP's built-in HTTP client), with vendored Waveshare drivers isolated in an epd static library.

## Hardware Requirements

- Raspberry Pi Pico W
- Waveshare 2.9" B (Tri-color) E-Paper HAT or breakout board
  - Note: This project uses the V3/V4 driver files (EPD_2in9b_V3).

## Wiring Guide

Based on the configuration in DEV_Config.c:

| Pico Pin | Waveshare Pin | Function     |
| -------- | ------------- | ------------ |
| GPIO 12  | RST           | Reset        |
| GPIO 8   | DC            | Data/Command |
| GPIO 9   | CS            | Chip Select  |
| GPIO 13  | BUSY          | Busy Signal  |
| GPIO 10  | SCK           | SPI Clock    |
| GPIO 11  | MOSI          | SPI Data     |
| 3V3      | VCC           | Power (3.3V) |
| GND      | GND           | Ground       |

Ensure your _DEV_Config.c_ pin definitions match your physical wiring.

## Software Architecture

### Project Structure

```text
weather_station/
├── CMakeLists.txt          # Build configuration
├── main.c                  # Entry point and main loop
├── config/
│   └── secrets.h           # WiFi credentials & location (GitIgnored)
├── src/
│   ├── wifi.c/h            # Wi-Fi initialization and connection
│   ├── network.c/h         # HTTP GET via lwIP's built-in http client
│   ├── weather.c/h         # Fetches and cleans wttr.in data
│   └── display.c/h         # Panel init, buffers, text layout, render
├── lib/
│   ├── Debug.h             # Waveshare display driver debugging
│   ├── EPD_2in9b_V3.c/h    # Waveshare display driver
│   ├── DEV_Config.c/h      # Low-level SPI/GPIO abstraction
│   ├── GUI_Paint.c/h       # Waveshare drawing library
│   ├── font16.c/fonts.h    # 16px bitmap font
│   └── lwipopts.h          # lwIP configuration
└── build/                  # Generated build files (GitIgnored)
```

### Key Components

The code is layered: main orchestrates, wifi/weather/display are independent domains, and network is a thin wrapper around lwIP's httpc_get_file_dns(). No layer reaches past its immediate dependency.

## Configuration

### 1. Location

Edit `config/secrets.h` t oset your latitude and longitude:

```C
#define LATITUDE  51.5074
#define LONGITUDE -0.1278
```

### 2. Wi-Fi Credentials

Edit `config/secrets.h` with your network details:

```C
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASS "YourPassword"
```

## Building and Flashing

### Prerequisites

- (Raspberry Pi Pico SDK)[https://github.com/raspberrypi/pico-sdk]
- CMake 3.13+
- ARM GCC toolchain (arm-none-eabi-gcc)

### Build commands

```bash
# Configure
cmake -S . -B build -DPICO_BOARD=pico_w -DPICO_SDK_PATH=/PATH/TO/pico-sdk/

# Build
cmake --build build -j$(nproc)
```

### Flashing

1. Hold the **BOOTSEL** button on the Pico W while plugging it into your computer.
2. Drage and drop `build/pico_weather_station.uf2` onto the RPI-RP2 drive.
3. The device will reboot and start the weather loop.

## Usage

1. **Power On:** The device connect to Wi-Fi.
2. **Fetch:** Retrieves weather data from wttr.in.
3. **Display:** Renders teh ACSII art on the e-paper screen.
4. **Sleep**: Enters a 1-hour sleep cycle.
5. **Repeat:** Wakes up and repeats.

Note: The e-paper display retains the image even when powered off, so the screen remains visible during the sleep cycle.

## Troubleshooting

### "Full Red" Screen

- Ensure DEV_Module_Init() is called only once.
- Check wiring for the `BUSY` pin.

### No Wi-Fi Connection

- Verify SSID/PASS in `secrets.h`

### Text Cut Off

- Rotate the display by chaning Paint_NewImage roation to 90 degrees.
- Try a smaller font from Waveshare

### License

This project uses components from the Raspberry Pi Pico SDK and Waveshare. Please respect the licensing terms of those libraries.

---

Built with blood, sweat, and tears using a Raspberry Pi Pico W
