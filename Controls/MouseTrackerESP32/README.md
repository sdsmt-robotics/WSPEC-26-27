# Mouse Tracker ESP32-S3

This is a PlatformIO Arduino scaffold for the `esp32-s3-devkitc-1` board.
Open this directory as the PlatformIO project and build/upload the
`esp32-s3-devkitc-1` environment. The serial monitor is configured for 115200
baud.

The existing `Controls/mouse_tracker.c++` is a Windows Raw Input program and
is intentionally unchanged; it cannot run on the ESP32-S3. This firmware
scaffold currently only prints a startup message. PAW3395 SPI support and
sensor pin assignments have not been implemented; choose and document the
board-specific SPI, chip-select, and interrupt wiring when adding the sensor
driver.
