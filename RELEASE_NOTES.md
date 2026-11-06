# v0.1.0-experimental — Native tablet pad for XP-Pen Artist 15.6 Pro V2

**Experimental community release — not an official XP-Pen driver.**

This source release fixes missing pad keys and dial support for the **XP-Pen Artist 15.6 Pro V2** (`28bd:096f`) on Linux with HID-BPF, as demonstrated with **Kubuntu 26.04 LTS, KDE Plasma on Wayland**.

### Verified on the original test device

- **All 8 express keys** work as native, assignable KDE tablet-pad buttons.
- **The dial/wheel** appears and works in the KDE Tablet Pad configuration.
- **Pen input and monitor mapping** remain functional.
- The HID-BPF source can be built with `udev-hid-bpf`; targeted permanent installation completed successfully.

### Install / build

Read the [README](README.md) before installing. This release deliberately ships **source code**, not a precompiled `.bpf.o`. Build it against an appropriate `udev-hid-bpf` source tree and use its single-file installer with `--install-exe`. Optional helper install/uninstall scripts are included for systems with the compiled loader at `~/udev-hid-bpf/builddir`.

### Limitations

This is a one-device test. Reboot persistence, other firmware revisions, and other distributions still need independent confirmation. On the original machine, KDE's dial action initially required multiple applications of the configuration before it responded. The report descriptor fixup intentionally requires the original **174-byte** composite descriptor.

If you have this exact model and USB ID, please [report your results](https://github.com/Shelin-boop/XPPEN_Artist_15.6_Pro_V2/issues).

The HID-BPF source is **GPL-2.0-only** and adapted from the upstream XP-Pen Deco 01 V3 program; attribution is included in the source.
