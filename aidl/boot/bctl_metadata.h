/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define BCTL_METADATA_PARTITION "/dev/block/bootdevice/by-name/slotinfo"
#define BCTL_METADATA_OFFSET 0x800
#define BCTL_METADATA_MAGIC 0x43425845

#define SLOT_SUFFIX_A "_a"
#define SLOT_SUFFIX_B "_b"

// Our internal data structures
struct slot_metadata_t {
    uint32_t magic;
    uint8_t bootable;
    uint8_t is_active;
    uint8_t boot_successful;
    uint8_t tries_remaining;
    uint8_t reserved[8];
};

struct bctl_metadata_t {
    slot_metadata_t slot_info[2];
};
