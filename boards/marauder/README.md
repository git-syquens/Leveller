# Marauder Board Assets

- Source: copied from `E:\Dev\Marauder` to reuse the integrated ESP32 board with built-in LCD/touch/SD/Wi-Fi.
- Config: root `sdkconfig` now matches the Marauder board; previous config saved as `sdkconfig.esp32c6.bak`. A copy also lives at `boards/marauder/sdkconfig`.
- Dependencies: `boards/marauder/idf_component.yml` plus the LCD/GUI/XPT2046 drivers under `boards/marauder/components/` (pin mappings defined in the headers).
- Docs: `boards/marauder/docs/` holds the pinout reference and the ESP32_CREEN PDF/PNGs for the board layout.
- Tooling: `.vscode/settings.json` is set to ESP32 (UART on COM8, esp-idf v5.5 paths, xtensa clangd query driver). Adjust `idf.portWin` if the board enumerates on another COM port.
