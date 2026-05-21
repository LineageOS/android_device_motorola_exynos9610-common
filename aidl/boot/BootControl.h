/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android-base/properties.h>
#include <aidl/android/hardware/boot/BnBootControl.h>

#include "bctl_metadata.h"

namespace aidl {
namespace android {
namespace hardware {
namespace boot {
namespace implementation {

using ::android::base::GetProperty;

class BootControl : public BnBootControl {
  public:
    // Methods from ::aidl::android::hardware::boot::IBootControl follow.
    ::ndk::ScopedAStatus getNumberSlots(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus getCurrentSlot(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus markBootSuccessful() override;
    ::ndk::ScopedAStatus setActiveBootSlot(int32_t slot) override;
    ::ndk::ScopedAStatus setSlotAsUnbootable(int32_t slot) override;
    ::ndk::ScopedAStatus isSlotBootable(int32_t slot, bool* _aidl_return) override;
    ::ndk::ScopedAStatus isSlotMarkedSuccessful(int32_t slot, bool* _aidl_return) override;
    ::ndk::ScopedAStatus getSuffix(int32_t slot, std::string* _aidl_return) override;
    ::ndk::ScopedAStatus getActiveBootSlot(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus getSnapshotMergeStatus(::aidl::android::hardware::boot::MergeStatus* _aidl_return) override;
    ::ndk::ScopedAStatus setSnapshotMergeStatus(::aidl::android::hardware::boot::MergeStatus in_status) override;

  private:
    std::string mBlkDevice = BCTL_METADATA_PARTITION;

    bool readMetadata(bctl_metadata_t& data);
    bool writeMetadata(bctl_metadata_t& data);
    bool validateMetadata(bctl_metadata_t& data);
    bctl_metadata_t resetMetadata();
};

}  // namespace implementation
}  // namespace boot
}  // namespace hardware
}  // namespace android
}  // namespace aidl
