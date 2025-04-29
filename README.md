# Plantar Pressure Distribution

## Materials
- ELEGOO ESP-WROOM-32 Board

## Setup
To program on **Arduino IDE**:
1. Install the `esp32` Board Manager from the left sidebar tab.
2. For Windows: under Files > Preferences and for Mac: under Arduino IDE > Settings, add `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` to "Additional Board URLs". If you already have that field populated, click the double-window icon to the right of the text entry field and add the new URL in a new row.
3. Install (if you don't have already), the [CP210x USB-to-Serial Driver](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads) for your operating system.
4. Select the Board `ESP32 Dev Module` under Tools > Board.
5. Select the port that the driver installed to. For me, it was COM5, but there should be limited options when you go to Tools > Port.

To program
