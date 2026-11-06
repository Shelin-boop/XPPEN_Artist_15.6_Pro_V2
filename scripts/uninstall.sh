#!/usr/bin/env bash
# Revert files newly installed by scripts/install.sh; preserve modified or unknown files.
set -Eeuo pipefail
(( EUID != 0 )) || { echo "Run without sudo; the script invokes sudo." >&2; exit 1; }
MANIFEST="${HOME}/.local/share/xppen-artist156prov2/installed-files.tsv"
[[ -s "$MANIFEST" ]] || { echo "No install manifest found: $MANIFEST; refusing to delete unknown files." >&2; exit 1; }
removed=0
skipped=0
while IFS=$'\t' read -r expected_hash path; do
    [[ -n "$path" ]] || continue
    case "$path" in
        /etc/udev-hid-bpf/*|/etc/udev/rules.d/*) ;;
        *) echo "Skipping unexpected path: $path" >&2; ((skipped+=1)); continue ;;
    esac
    [[ -f "$path" ]] || continue
    actual_hash=$(sudo sha256sum -- "$path" | awk '{print $1}')
    if [[ "$actual_hash" != "$expected_hash" ]]; then
        echo "Skipping modified file: $path" >&2
        ((skipped+=1))
        continue
    fi
    echo "Removing: $path"
    sudo rm -- "$path"
    ((removed+=1))
done < "$MANIFEST"
sudo udevadm control --reload
if (( skipped == 0 )); then
    mv "$MANIFEST" "${MANIFEST}.removed-$(date +%Y%m%d-%H%M%S)"
else
    echo "Some files were skipped. The manifest remains available for review." >&2
fi
echo "Removed: $removed; skipped: $skipped. Unplug/replug the tablet USB data cable."
