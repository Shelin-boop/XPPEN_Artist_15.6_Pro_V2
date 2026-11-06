# Testing checklist — Artist 15.6 Pro V2

**This is experimental software that modifies the Linux HID input interface.** It has been verified on one device only.

## Hardware compatibility

1. Confirm your tablet is exactly **XP-Pen Artist 15.6 Pro V2**.
2. Confirm USB ID: `lsusb -d 28bd:096f`.
3. Confirm your kernel has HID-BPF support. This was tested on Linux `7.0.0-42-generic` with libinput `1.31.1`, KDE Plasma/Wayland.
4. Confirm the composite interface's original HID descriptor is **174 bytes**. The program only runs on this specific descriptor shape.

## After installing and reconnecting the USB data cable

Run `sudo libinput list-devices`. Locate the **XPPen Artist 15.6 Pro V2 Keypad** entry and check:
- `Capabilities: tablet-pad`
- `Buttons: 0, 1, 2, 3, 4, 5, 6, 7`

In KDE Plasma: **System Settings → Drawing Tablet → Pad**.
- [ ] Eight configurable pad buttons are shown.
- [ ] Physical keys 1 through 8 each trigger the action assigned to them.
- [ ] The dial appears.
- [ ] Dial clockwise/counterclockwise trigger independent relative increments.
- [ ] The pen still moves, presses and maps to the intended monitor.
- [ ] No unwanted pointer/keyboard regression.

If a control does not respond, identify the currently enumerated `/dev/input/eventN` of the **Keypad** device using `libinput list-devices`; then run `sudo libinput debug-events --device /dev/input/eventN`. Watch for the pad events while pressing buttons and turning the wheel; exit with Ctrl+C. **Event numbers are not stable** across reconnects/reboots.

## Persistence

**A working replug is not proof of a working reboot.**

- [ ] Reboot the machine completely.
- [ ] Do **not** run a manual `udev-hid-bpf add` command.
- [ ] Verify `tablet-pad` capability again.
- [ ] Test all eight keys and both directions of the dial.
- [ ] Confirm the pen still maps to the intended display.

## If anything goes wrong

Unplug/replug the USB **data** cable to remove a manually attached patch. For a permanently installed patch, use the installation's corresponding uninstall procedure, reload udev rules and reconnect the tablet. The helper uninstaller in this repository removes only tracked, unchanged, tablet-specific files. It deliberately **preserves the shared `udev-hid-bpf` executable** to avoid breaking unrelated devices.

Report details at the [issue tracker](https://github.com/Shelin-boop/XPPEN_Artist_15.6_Pro_V2/issues).
