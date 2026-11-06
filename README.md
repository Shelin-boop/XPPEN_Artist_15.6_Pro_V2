# XP-Pen Artist 15.6 Pro V2 — Native pad support on Linux/Wayland

**Status: experimental; successfully tested on one physical device.**

This project provides a HID-BPF quirk for the **XP-Pen Artist 15.6 Pro V2**, USB **VID:PID `28bd:096f`**. It makes the **eight express keys and dial** available as a native tablet pad in KDE Plasma on Wayland, without XP-Pen's proprietary configuration application or a third-party key remapper.

## Current public release

**[v0.1.0-experimental](https://github.com/Shelin-boop/XPPEN_Artist_15.6_Pro_V2/releases/tag/v0.1.0-experimental)** — experimental source release (not an upstream-approved fix). GitHub automatically offers source archives for published tags. There is **no precompiled BPF object** in this release because kernel and loader compatibility must be considered.

See the [changelog](CHANGELOG.md), [test checklist](docs/TESTING.md) and [contribution guide](CONTRIBUTING.md) before deploying or reporting results.

## Verified hardware and software

- XP-Pen Artist **15.6 Pro V2** (`28bd:096f`).
- Kubuntu **26.04 LTS**; kernel **7.0.0-42-generic**; libinput **1.31.1**; KDE Plasma / Wayland.
- libinput exposes a `tablet-pad` device with eight buttons.
- All **eight** physical keys work and can be configured through **System Settings → Drawing Tablet → Pad**.
- The **dial** appears in the KDE settings and worked after applying its mapping several times. The reason for this initial delay has not yet been determined.
- The **pen remains functional**, including the display mapping in the tested multi-monitor setup.

The quirk has **not** been verified on a second tablet, other firmware revisions, or after a clean reboot with automatic loading.

## What the patch does

By default, the device sends the pad controls as **keyboard Report ID 6** on a composite mouse/keyboard interface; the pen is provided on a separate HID interface. As a result, libinput does not identify the keys or wheel as tablet-pad controls.

The HID-BPF program changes **only the composite mouse/keyboard interface with the original 174-byte HID report descriptor**. It retains the existing mouse collections, replaces the keyboard collection with a tablet-pad collection, and translates the eight key reports and dial direction reports into native pad events. The separate pen interface is not modified.

| Control | Original report | Native behavior |
| --- | --- | --- |
| Key 1 | `06 00 05 …` | Pad button 1 |
| Key 2 | `06 00 08 …` | Pad button 2 |
| Key 3 | `06 04 00 …` | Pad button 3 |
| Key 4 | `06 00 2c …` | Pad button 4 |
| Key 5 | `06 01 16 …` | Pad button 5 |
| Key 6 | `06 01 1d …` | Pad button 6 |
| Key 7 | `06 01 2e …` | Pad button 7 |
| Key 8 | `06 01 2d …` | Pad button 8 |
| Dial backward | `06 01 56 …` | Relative dial −1 |
| Dial forward | `06 01 57 …` | Relative dial +1 |

## Building with `udev-hid-bpf`

This patch is intended for Linux kernels with HID-BPF support and the [udev-hid-bpf](https://gitlab.freedesktop.org/libevdev/udev-hid-bpf/) build system. The following steps **compile** the source but do not attach it to a device:

```bash
git clone https://gitlab.freedesktop.org/libevdev/udev-hid-bpf.git
git clone https://github.com/Shelin-boop/XPPEN_Artist_15.6_Pro_V2.git
cd udev-hid-bpf

cp ../XPPEN_Artist_15.6_Pro_V2/src/0010-XPPen__Artist156ProV2_experimental.bpf.c src/bpf/testing/

python3 - <<'PY'
from pathlib import Path
p = Path("src/bpf/testing/meson.build")
s = p.read_text()
entry = "    '0010-XPPen__Artist156ProV2_experimental.bpf.c',\n"
anchor = "sources = [\n"
assert anchor in s, "Unsupported Meson source layout"
if entry not in s:
    p.write_text(s.replace(anchor, anchor + entry, 1))
PY

meson setup builddir -Dbpfs=testing
meson compile -C builddir
```

For a targeted **manual test** of the compiled program, locate the 174-byte interface and use the upstream loader's `add` command. Unplug/replug the USB **data cable** to discard a manually attached BPF program.

For **permanent installation**, the upstream `udev-hid-bpf` tool documents a single-program installer that creates a udev rule:

```bash
./builddir/udev-hid-bpf install --install-exe ./builddir/src/bpf/0010-XPPen__Artist156ProV2_experimental.bpf.o
sudo udevadm control --reload
```

The `--install-exe` flag is important: the generated udev rule expects the loader executable under its configured prefix (typically `/usr/local/bin/udev-hid-bpf`). Without it, the command may fail even if a distro has a separate loader in `/usr/bin`. You may also use the `scripts/install.sh` and `scripts/uninstall.sh` helper scripts on a host with the compiled loader in `~/udev-hid-bpf/builddir`. Run them as your normal user (they invoke `sudo` when needed). **The uninstaller preserves the potentially shared `/usr/local/bin/udev-hid-bpf` executable**, even if this script installed it, to avoid breaking HID-BPF support for other devices. The uninstaller removes only tracked, unchanged tablet-specific files.

Afterward, unplug/replug the tablet's USB data cable, then verify with `sudo libinput list-devices` (look for `tablet-pad`) and test all controls in KDE. Installation modifies the host's udev configuration; inspect the installed files before removing or replacing them.

Detailed instructions: [official udev-hid-bpf documentation](https://libevdev.pages.freedesktop.org/udev-hid-bpf/getting-started.html).

## Status and feedback

This is **not** an official XP-Pen driver or an upstream-approved kernel quirk. Testing is limited to a single machine. We welcome reports from other owners with **exactly the same USB VID:PID**, ideally including the kernel version, firmware revision (if known), whether pen/display mapping works, eight-key tests, and dial behavior after a fresh reboot.

See [the upstream submission proposal](docs/UPSTREAM_ISSUE_EN.md) for technical context and pending test cases.

## License and credit

The source code carries **SPDX-License-Identifier: GPL-2.0-only**. It is adapted from the XP-Pen Deco 01 V3 HID-BPF patch (Red Hat, 2025); see the source header for details.
