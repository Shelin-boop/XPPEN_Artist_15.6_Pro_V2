# Contributing

Thanks for helping improve native Linux support for the **XP-Pen Artist 15.6 Pro V2**.

## Testing another device

Only test this patch on **USB VID:PID `28bd:096f`** and on a system with HID-BPF support. Back up custom KDE tablet settings before trying experimental revisions.

Please include these details in a [new GitHub issue](../../issues/new):
- Linux distribution and version, kernel (`uname -r`), KDE Plasma version, Wayland/X11, and libinput version.
- Exact model and `lsusb -d 28bd:096f` result.
- Results for **each of the eight express keys**, **both wheel directions**, and **pen/display mapping**.
- Whether loading works again after a **clean reboot** without running the loader manually.
- Whether you observed any regression (mouse, keyboard, pen pressure/tilt, display mapping).

If attaching traces, check them for sensitive data first. Raw HID keyboard reports may contain key presses or shortcuts; never record while typing passwords or private messages. Remove device serial numbers, usernames, private paths, and any unrelated information.

## Code changes

The BPF program lives in `src/0010-XPPen__Artist156ProV2_experimental.bpf.c`. Keep its exact USB and HID-descriptor checks. Avoid touching the separate pen interface.

For upstream integration, use [libevdev/udev-hid-bpf](https://gitlab.freedesktop.org/libevdev/udev-hid-bpf/) as the base and add the BPF source to `src/bpf/testing/meson.build`. Prefer small, testable changes with before/after `libinput` output and references to the matching descriptor.

## License

Code contributions are expected to be compatible with **GPL-2.0-only**. Preserve original SPDX and attribution lines in adapted files.

## Project scope

This is a community project, not an official XP-Pen driver. No proprietary XP-Pen software or third-party key remappers are needed for the tested KDE configuration.
