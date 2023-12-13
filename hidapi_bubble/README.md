# bubble display driver with hidapi

7-segment 8-digit LED bubble display with hidapi control

## Gotchas

So you are aware, there are a number of gotchas when making platform-agostic HID RAW devices.
 * The first byte of the requests must be the feature ID. (this demo uses 0xaa)
 * Windows REQUIRES notification in the HID descritor of the feature match both ID and transfer size!
 * Yes, that's annoying because it means on Windows at least all your messages must be the same size.
 * On Linux, you may need to either be part of plugdev, and/or have your udev rules include the following:
```
KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0664", GROUP="plugdev"
```
