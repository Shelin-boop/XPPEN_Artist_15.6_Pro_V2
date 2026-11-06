#!/usr/bin/env bash
# XP-Pen Artist 15.6 Pro V2 (28bd:096f) - single-quirk installation
# Runs as an ordinary user; invokes sudo for installation and udev reload.
set -Eeuo pipefail
ROOT="${HOME}/udev-hid-bpf"
NAME="0010-XPPen__Artist156ProV2_experimental.bpf.o"
OBJ="${ROOT}/builddir/src/bpf/${NAME}"
LOADER="${ROOT}/builddir/udev-hid-bpf"
STATE="${HOME}/.local/share/xppen-artist156prov2"
MANIFEST="${STATE}/installed-files.tsv"
(( EUID != 0 )) || { echo "Run without sudo; the script invokes sudo." >&2; exit 1; }
[[ -s "$OBJ" && -x "$LOADER" ]] || { echo "Missing compiled BPF file or loader: $OBJ / $LOADER" >&2; exit 1; }
[[ ! -e "$MANIFEST" ]] || { echo "Installation manifest already exists: $MANIFEST" >&2; exit 1; }
mkdir -p "$STATE"
chmod 700 "$STATE"
TMP=$(mktemp -d)
trap 'rm -rf -- "$TMP"' EXIT
list_config() {
    for dir in /etc/udev-hid-bpf /etc/udev/rules.d; do
        if [[ -d "$dir" ]]; then
            sudo find "$dir" -maxdepth 1 -type f -print
        fi
    done | LC_ALL=C sort -u
}
echo "=== Installation preview ==="
sudo "$LOADER" install --dry-run "$OBJ"
for dir in /etc/udev-hid-bpf /etc/udev/rules.d; do
    if [[ -d "$dir" ]] && sudo grep -lRFq -- "$NAME" "$dir" 2>/dev/null; then
        echo "Patch appears already installed under $dir. Refusing duplicate installation." >&2
        exit 1
    fi
done
list_config > "$TMP/before"
echo "=== Installing only $NAME ==="
sudo "$LOADER" install "$OBJ"
sudo udevadm control --reload
list_config > "$TMP/after"
comm -13 "$TMP/before" "$TMP/after" > "$TMP/new"
: > "$MANIFEST"
chmod 600 "$MANIFEST"
while IFS= read -r path; do
    if [[ "$path" == "/etc/udev-hid-bpf/$NAME" ]] ||
       [[ "$(basename "$path")" == *Artist156ProV2* ]] ||
       sudo grep -Fq -- "$NAME" "$path" 2>/dev/null; then
        hash=$(sudo sha256sum -- "$path" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$path" >> "$MANIFEST"
        echo "Installed: $path"
    fi
done < "$TMP/new"
if [[ ! -s "$MANIFEST" ]]; then
    echo "WARNING: No newly created config files were recorded; inspect the loader output before attempting uninstall." >&2
else
    echo "Manifest: $MANIFEST"
fi
echo "Unplug/replug the tablet USB data cable. Check 'sudo libinput list-devices' and all pad controls in KDE."
