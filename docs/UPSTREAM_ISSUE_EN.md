# [HID-BPF] XPPen Artist 15.6 Pro V2 (28bd:096f): convert express keys and dial to native tablet pad

This is a proposal for a new HID-BPF quirk for the XPPen Artist 15.6 Pro V2, USB vendor/product ID `28bd:096f`.

## Problem

Under KDE Plasma on Wayland, the pen is supported and display mapping works, but the 8 express keys and the dial are reported through the composite mouse/keyboard HID interface as keyboard shortcuts, so KDE does not expose the Pad configuration. The vendor's proprietary Linux driver enables button configuration but interferes with the multi-monitor pen mapping on the tested setup.

## Hardware and tested environment

- XPPen Artist 15.6 Pro V2, `28bd:096f` (USB)
- Kubuntu 26.04 LTS, KDE Plasma on Wayland
- Linux `7.0.0-42-generic`, libinput `1.31.1`
- `udev-hid-bpf` source tree `2.3.0-20260703` (built locally)
- HID reports collected with `hid-recorder`; original composite mouse/keyboard report descriptor: **174 bytes**, keyboard Report ID **6**.

## Patch

Attached: `0010-XPPen__Artist156ProV2_experimental.bpf.c` (GPL-2.0-only, adapted from the XPPen Deco 01 V3 HID-BPF program, 2025 Red Hat).

The fix targets only the 174-byte composite mouse/keyboard interface. It replaces the keyboard collection with a tablet pad descriptor and converts eight keyboard shortcut signals into tablet-pad button bits. The dial is converted from Ctrl+KP+/KP- into relative wheel motion. The separate pen HID interface is not modified.

The eight button raw codes are `05`, `08`, `Alt modifier bit 2`, `2c`, `16`, `1d`, `2e`, `2d`. The dial emits `Ctrl+KP-` (`56`) / `Ctrl+KP+` (`57`), independently from the final two keys (`Ctrl+-` / `Ctrl+=`).

## Verified results on one device

With the HID-BPF program manually loaded:

- libinput reports `XPPen ... Artist 15.6 Pro V2 Keypad` with `Capabilities: tablet-pad`, 8 buttons (`0..7`)
- KDE Drawing Tablet settings display the **Pad** page
- All 8 physical express keys can be assigned and work
- The dial is exposed and configurable in KDE, and worked after reapplying its configuration (needed more than one apply during this test)
- Pen remains detected as `tablet`; mapping to the integrated tablet display still works

This is a **single-machine test**, not a claim of cross-firmware compatibility. Additional testers and review are welcome. The BPF descriptor fixup guards against changes in the 174-byte composite layout, and the other HID interfaces should be excluded by the `probe()` logic.

## Request

Would maintainers be willing to review this as a new `src/bpf/testing` quirk? I can provide sanitized raw HID descriptors and `hid-recorder` traces, run requested verification commands, and test revised patches on the hardware.

## Follow-up tests requested before merging

- Replug, fresh reboot and repeat KDE configuration
- Simultaneous key presses, dial speed/direction and button release behavior
- Pen button/pressure/tilt and calibrated dual-monitor mapping
- Test on another Artist 15.6 Pro V2 / firmware version if available
- Check BPF verifier and source coding conventions; consider kernel HID-BPF selftests

Reference project: https://gitlab.freedesktop.org/libevdev/udev-hid-bpf/