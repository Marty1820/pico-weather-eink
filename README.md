# PicoW future Weather Station

Will eventually connect to network download weather from openmeteo and aqi from aqicn then display it on a Waveshare ePaper 2.9B display

## How to compile

`cmake` (I know I tried to go without but couldn't figure out pico-sdk

```bash
cmake -S . -B build -DPICO_BOARD=pico_w -DPICO_SDK_PATH=~/Projects/pico/pico_c/pico-sdk/
```

*Change the board and sdk-path to proper locations

```bash
cmake --build build -j8
```

Copy the file `./build/file_name.utf2` to the pico

connect to console with `screen /dev/ttyACM0 115200`
