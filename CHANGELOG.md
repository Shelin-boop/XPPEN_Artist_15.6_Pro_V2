# Changelog

## [v0.1.0-experimental] — 2026-11-06

First public **experimental source release** for the XP-Pen Artist 15.6 Pro V2 (USB `28bd:096f`).

### Confirmed on one test device
- Eight physical express keys appear and work as native tablet-pad buttons in KDE Plasma on Wayland.
- Dial/wheel appears in KDE's Pad settings and responds after assigning its action.
- Pen remains functional; pen-to-display mapping remains managed by KDE in the tested dual-monitor setup.
- HID-BPF patch can be built with `udev-hid-bpf` and loaded on the 174-byte composite keyboard/mouse interface.
- Single-program permanent installer completed successfully; automatic reloading after a **fresh reboot** has **not yet been independently confirmed**.

### Included
- GPL-2.0-only HID-BPF source, based on the XP-Pen Deco 01 V3 quirk.
- Build, installation and removal instructions.
- Optional helper installer/uninstaller scripts.
- Technical upstream report, test checklist and issue template.

### Known limitations
- Tested on **one physical device** only; firmware and other Linux distribution support are unknown.
- The dial initially required multiple KDE configuration applications.
- No precompiled BPF binary is provided: build from source for your kernel/loader.
- The scripts have been reviewed for syntax but have not been independently tested across Linux distributions.

For changes, patches or test reports, open a GitHub issue.
