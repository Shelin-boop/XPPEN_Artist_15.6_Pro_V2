// SPDX-License-Identifier: GPL-2.0-only
/* Based on the Deco 01 V3 HID-BPF program (c) 2025 Red Hat.
 * Artist 15.6 Pro V2 adaptation: experimental, 2026.
 */

#include "vmlinux.h"
#include "hid_bpf.h"
#include "hid_bpf_helpers.h"
#include "hid_report_helpers.h"
#include <bpf/bpf_tracing.h>

#define VID_UGEE 0x28BD /* VID is shared with SinoWealth and Glorious and prob others */
#define PID_ARTIST_15_6_PRO_V2 0x096f

HID_BPF_CONFIG(
	HID_DEVICE(BUS_USB, HID_GROUP_GENERIC, VID_UGEE, PID_ARTIST_15_6_PRO_V2),
);

/*
 * Experimental: XPPen Artist 15.6 Pro V2 (USB 28bd:096f).
 * The specific 174-byte HID descriptor was captured from the user's device.
 * Report ID 6 is an 8-byte keyboard report. Mouse reports 1 and 9 are kept.
 * No modifications are made to the separate pen and vendor HID interfaces.
 *
 * Confirmed from actual hid-recorder capture for this model:
 *   K1..K6: 05 08 Alt 2c 16 1d (K3 sets bit 2 in modifier byte)
 *   K7: Ctrl + '='  (keyboard usage 0x2e)
 *   K8: Ctrl + '-'  (keyboard usage 0x2d)
 *   Dial: Ctrl + KP+ / KP- (usages 0x57 / 0x56), distinct from K7/K8.
 * Report ID 6 stays 8 bytes. The dial uses a relative REL_WHEEL event.
 * EXPERIMENTAL: compile and test before any permanent installation.
 */

/* 174-byte mouse+keyboard interface from this specific Artist 15.6 Pro V2.
 * Keep its two mouse reports unchanged; substitute only the final keyboard
 * application collection (report ID 6) with a native tablet-pad collection.
 * The separate pen HID interface is NEVER modified.
 */
#define ORIGINAL_COMPOSITE_RDESC_LENGTH 174
#define ORIGINAL_MOUSE_PREFIX_LENGTH 134
#define PAD_REPORT_LENGTH 8
#define PAD_REPORT_ID 6
#define PAD_NUM_BUTTONS 8

static const __u8 original_mouse_prefix[] = {
	0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x85, 0x09, 0x09, 0x01, 0xa1, 0x00,
	0x05, 0x09, 0x19, 0x01, 0x29, 0x05, 0x15, 0x00, 0x25, 0x01, 0x95, 0x05,
	0x75, 0x01, 0x81, 0x02, 0x95, 0x03, 0x81, 0x01, 0x05, 0x01, 0x09, 0x30,
	0x09, 0x31, 0x26, 0xff, 0x7f, 0x95, 0x02, 0x75, 0x10, 0x81, 0x02, 0x09,
	0x38, 0x15, 0x81, 0x25, 0x7f, 0x95, 0x01, 0x75, 0x08, 0x81, 0x06, 0xc0,
	0xc0, 0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x09, 0x01, 0xa1, 0x00, 0x85,
	0x01, 0x05, 0x09, 0x19, 0x01, 0x29, 0x05, 0x95, 0x05, 0x75, 0x01, 0x15,
	0x00, 0x25, 0x01, 0x81, 0x02, 0x95, 0x03, 0x81, 0x01, 0x05, 0x01, 0x09,
	0x30, 0x09, 0x31, 0x95, 0x02, 0x75, 0x08, 0x15, 0x81, 0x25, 0x7f, 0x81,
	0x06, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x95, 0x01, 0x75, 0x08, 0x81,
	0x06, 0x05, 0x0c, 0x0a, 0x38, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x06,
	0xc0, 0xc0,
};

static const __u8 fixed_rdesc_pad[] = {
	UsagePage_GenericDesktop
	Usage_GD_Keypad
	CollectionApplication(
		// Byte 0 in report is the report ID
		ReportId(PAD_REPORT_ID)
		ReportCount(1)
		ReportSize(8)
		UsagePage_Digitizers
		Usage_Dig_TabletFunctionKeys
		CollectionPhysical(
			// Byte 1 is the button state
			UsagePage_Button
			UsageMinimum_i8(0x01)
			UsageMaximum_i8(PAD_NUM_BUTTONS)
			LogicalMinimum_i8(0x0)
			LogicalMaximum_i8(0x1)
			ReportCount(PAD_NUM_BUTTONS)
			ReportSize(1)
			Input(Var|Abs)
			// Byte 2 in report - just exists so we get to be a tablet pad
			UsagePage_Digitizers
			Usage_Dig_BarrelSwitch // BTN_STYLUS
			ReportCount(1)
			ReportSize(1)
			Input(Var|Abs)
			ReportCount(7) // padding
			Input(Const)
			// Bytes 3/4 in report - just exists so we get to be a tablet pad
			UsagePage_GenericDesktop
			Usage_GD_X
			Usage_GD_Y
			ReportCount(2)
			ReportSize(8)
			Input(Var|Abs)
			// Byte 5: relative dial (+1 / -1), maps to REL_WHEEL
			Usage_GD_Wheel
			LogicalMinimum_i8(-1)
			LogicalMaximum_i8(1)
			ReportCount(1)
			ReportSize(8)
			Input(Var|Rel)
			// Bytes 6 and 7 are unused; total input report is still 8 bytes.
			ReportCount(2)
			ReportSize(8)
			Input(Const)
		)
	)
};

SEC(HID_BPF_RDESC_FIXUP)
int BPF_PROG(xppen_artist156prov2_rdesc_fixup, struct hid_bpf_ctx *hctx)
{
	__u8 *data = hid_bpf_get_data(hctx, 0, HID_MAX_DESCRIPTOR_SIZE);
	if (!data || hctx->size != ORIGINAL_COMPOSITE_RDESC_LENGTH)
		return 0;

	/* Fail closed if a future firmware has a different collection layout. */
	const __u8 keyboard_start[] = {0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x85, 0x06};
	if (__builtin_memcmp(&data[ORIGINAL_MOUSE_PREFIX_LENGTH],
			     keyboard_start, sizeof(keyboard_start)) != 0)
		return 0;

	__builtin_memcpy(data, original_mouse_prefix, sizeof(original_mouse_prefix));
	__builtin_memcpy(data + sizeof(original_mouse_prefix),
			 fixed_rdesc_pad, sizeof(fixed_rdesc_pad));
	return sizeof(original_mouse_prefix) + sizeof(fixed_rdesc_pad);
}

SEC(HID_BPF_DEVICE_EVENT)
int BPF_PROG(xppen_artist156prov2_device_event, struct hid_bpf_ctx *hctx)
{
	static const __u8 pad_buttons[] = { 0x05, 0x08, 0x00, 0x2c, 0x16, 0x1d, 0x2e, 0x2d };
	__u8 *data = hid_bpf_get_data(hctx, 0 /* offset */, PAD_REPORT_LENGTH /* size */);

	if (!data)
		return 0; /* EPERM check */

	if (data[0] == PAD_REPORT_ID) {
		__u8 button_mask = 0;
		__u8 wheel = 0;
		size_t d, b;

		/* data[1] stores the status of BTN_2 in the 3rd bit*/
		if (data[1] & BIT(2))
			button_mask |= BIT(2);

		/* The rest of the descriptor stores the buttons as in pad_buttons */
		for (d = 2; d < 8; d++) {
			/* Physical dial is separate from shortcut keys K7/K8. */
			if (data[1] & BIT(0)) {
				if (data[d] == 0x57)
					wheel = 1;
				else if (data[d] == 0x56)
					wheel = 0xff; /* -1 as a signed 8-bit delta */
			}
			for (b = 0; b < sizeof(pad_buttons); b++) {
				if (data[d] != 0 && data[d] == pad_buttons[b])
					button_mask |= BIT(b);
			}
		}

		/* ID, buttons, barrel placeholder, x, y, REL_WHEEL, reserved x2. */
		__u8 report[8] = {PAD_REPORT_ID, button_mask, 0, 0, 0, wheel, 0, 0};

		__builtin_memcpy(data, report, sizeof(report));
	}
	return 0;
}

HID_BPF_OPS(xppen_artist156prov2) = {
	.hid_rdesc_fixup = (void *)xppen_artist156prov2_rdesc_fixup,
	.hid_device_event = (void *)xppen_artist156prov2_device_event,
};

SEC("syscall")
int probe(struct hid_bpf_probe_args *ctx)
{
	ctx->retval = ctx->rdesc_size == ORIGINAL_COMPOSITE_RDESC_LENGTH ? 0 : -EINVAL;
	return 0;
}

char _license[] SEC("license") = "GPL";