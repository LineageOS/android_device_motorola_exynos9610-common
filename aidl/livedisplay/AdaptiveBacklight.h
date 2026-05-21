/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/livedisplay/BnAdaptiveBacklight.h>

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {

class AdaptiveBacklight : public BnAdaptiveBacklight {
  public:
    ndk::ScopedAStatus getEnabled(bool* _aidl_return) override;
    ndk::ScopedAStatus setEnabled(bool enabled) override;
};

}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
