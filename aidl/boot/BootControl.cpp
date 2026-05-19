/*
 * Copyright (C) 2020 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "BootControl.h"

#include <fstream>

namespace aidl {
namespace android {
namespace hardware {
namespace boot {
namespace implementation {

bool BootControl::readMetadata(bctl_metadata_t& data) {
    std::fstream in(mBlkDevice, std::ios::binary | std::ios::in);

    if (in.fail()) {
        return false;
    }

    in.seekg(BCTL_METADATA_OFFSET);

    if (in.fail()) {
        return false;
    }

    in.read(reinterpret_cast<char*>(&data), sizeof(bctl_metadata_t));

    if (!validateMetadata(data)) {
#if 0
        /* this is what samsung does, probably not what we desire */
        data = resetMetadata();
        if (validateMetadata(data)) {
            return true;
        }
#endif
        return false;
    }

    return !in.eof() && !in.fail();
}

bool BootControl::writeMetadata(bctl_metadata_t& data) {
    if (!validateMetadata(data)) return false;

    // We use std::ios::in | std::ios::out even though we only write so that
    // we don't truncate or append to the file, but rather overwrite the file
    // in the exact place that we want to write the struct to.
    std::fstream out(mBlkDevice, std::ios::binary | std::ios::in | std::ios::out);

    if (out.fail()) {
        return false;
    }

    out.seekp(BCTL_METADATA_OFFSET);

    if (out.fail()) {
        return false;
    }

    out.write(reinterpret_cast<const char*>(&data), sizeof(bctl_metadata_t));

    return !out.eof() && !out.fail();
}

bool BootControl::validateMetadata(bctl_metadata_t& data) {
    if (data.slot_info[0].magic != BCTL_METADATA_MAGIC ||
        data.slot_info[1].magic != BCTL_METADATA_MAGIC) {
        return false;
    }

    return true;
}

bctl_metadata_t BootControl::resetMetadata() {
    bctl_metadata_t data{};

    // reset to defaults
    data.slot_info[0].magic = BCTL_METADATA_MAGIC;
    data.slot_info[0].bootable = 1;
    data.slot_info[0].is_active = 1;
    data.slot_info[0].boot_successful = 0;
    data.slot_info[0].tries_remaining = 7;

    data.slot_info[1].magic = BCTL_METADATA_MAGIC;
    data.slot_info[1].bootable = 1;
    data.slot_info[1].is_active = 0;
    data.slot_info[1].boot_successful = 0;
    data.slot_info[1].tries_remaining = 7;

    return data;
}

// Methods from ::aidl::android::hardware::boot::IBootControl follow.
::ndk::ScopedAStatus BootControl::getNumberSlots(int32_t* _aidl_return) {
    *_aidl_return = 2;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus BootControl::getCurrentSlot(int32_t* _aidl_return) {
    bctl_metadata_t data;
    std::string slot_suffix = GetProperty("ro.boot.slot_suffix", "");

    if (!slot_suffix.empty()) {
        if (slot_suffix.compare(SLOT_SUFFIX_A) == 0) {
            *_aidl_return = 0;
            return ::ndk::ScopedAStatus::ok();
        } else if (slot_suffix.compare(SLOT_SUFFIX_B) == 0) {
            *_aidl_return = 1;
            return ::ndk::ScopedAStatus::ok();
        }
    } else {
        // read current slot from metadata incase "ro.boot.slot_suffix" is empty
        if (readMetadata(data)) {
            // This is a clever hack because if slot b is active,
            // is_active will be 0 and if slot a is active, is_active
            // will be 1. In other words, the "not" value of slot A's
            // is_active var lines up to the current active slot index.
            *_aidl_return = !data.slot_info[0].is_active;
            return ::ndk::ScopedAStatus::ok();
        }
    }

    // fallback to slot A
    *_aidl_return = 0;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus BootControl::markBootSuccessful() {
    bctl_metadata_t data;
    int32_t active_slot;

    getCurrentSlot(&active_slot);

    if (readMetadata(data)) {
        data.slot_info[active_slot].boot_successful = 1;
        data.slot_info[active_slot].tries_remaining = 0;

        if (writeMetadata(data)) {
            return ::ndk::ScopedAStatus::ok();
        } else {
            return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Failed to write metadata");
        }
    } else {
        return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Failed to read metadata");
    }
}

::ndk::ScopedAStatus BootControl::setActiveBootSlot(int32_t slot) {
    bctl_metadata_t data;

    if (slot < 2) {
        if (readMetadata(data)) {
            data.slot_info[slot].bootable = 1;
            data.slot_info[slot].is_active = 1;
            data.slot_info[slot].boot_successful = 0;
            data.slot_info[slot].tries_remaining = 7;

            data.slot_info[!slot].bootable = 1;
            data.slot_info[!slot].is_active = 0;
            data.slot_info[!slot].boot_successful = 0;
            data.slot_info[!slot].tries_remaining = 7;

            if (writeMetadata(data)) {
                return ::ndk::ScopedAStatus::ok();
            } else {
                return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Failed to write metadata");
            }
        } else {
            return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Failed to read metadata");
        }
    } else {
        return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Invalid slot");
    }
}

::ndk::ScopedAStatus BootControl::setSlotAsUnbootable(int32_t slot) {
    bctl_metadata_t data;

    if (slot < 2) {
        if (readMetadata(data)) {
            data.slot_info[slot].bootable = 0;

            if (writeMetadata(data)) {
                return ::ndk::ScopedAStatus::ok();
            } else {
                return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Failed to write metadata");
            }
        } else {
            return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Failed to read metadata");
        }
    } else {
        return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Invalid slot");
    }
}

::ndk::ScopedAStatus BootControl::isSlotBootable(int32_t slot, bool* _aidl_return) {
    bctl_metadata_t data;

    if (slot < 2) {
        if (readMetadata(data)) {
            *_aidl_return = data.slot_info[slot].bootable;
            return ::ndk::ScopedAStatus::ok();
        } else {
            *_aidl_return = false;
            return ::ndk::ScopedAStatus::ok();
        }
    } else {
        return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Invalid slot");
    }
}

::ndk::ScopedAStatus BootControl::isSlotMarkedSuccessful(int32_t slot, bool* _aidl_return) {
    bctl_metadata_t data;

    if (slot < 2) {
        if (readMetadata(data)) {
            *_aidl_return = data.slot_info[slot].boot_successful;
            return ::ndk::ScopedAStatus::ok();
        } else {
            *_aidl_return = false;
            return ::ndk::ScopedAStatus::ok();
        }
    } else {
        return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(0, "Invalid slot");
    }
}

::ndk::ScopedAStatus BootControl::getSuffix(int32_t slot, std::string* _aidl_return) {
    if (slot < 2) {
        if (slot == 0) {
            *_aidl_return = SLOT_SUFFIX_A;
        } else {
            *_aidl_return = SLOT_SUFFIX_B;
        }
    } else {
        // default to slot A
        *_aidl_return = SLOT_SUFFIX_A;
    }

    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus BootControl::getActiveBootSlot(int32_t* _aidl_return) {
    return getCurrentSlot(_aidl_return);
}

::ndk::ScopedAStatus BootControl::getSnapshotMergeStatus(::aidl::android::hardware::boot::MergeStatus* _aidl_return) {
    *_aidl_return = ::aidl::android::hardware::boot::MergeStatus::NONE;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus BootControl::setSnapshotMergeStatus(::aidl::android::hardware::boot::MergeStatus in_status) {
    (void)in_status;
    return ::ndk::ScopedAStatus::ok();
}

}  // namespace implementation
}  // namespace boot
}  // namespace hardware
}  // namespace android
}  // namespace aidl
