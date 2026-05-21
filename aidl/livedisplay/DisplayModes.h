/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/livedisplay/BnDisplayModes.h>

#include <map>

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {

class DisplayModes : public BnDisplayModes {
  public:
    DisplayModes();

    // Methods from ::aidl::vendor::lineage::livedisplay::BnDisplayModes follow.
    ndk::ScopedAStatus getDisplayModes(std::vector<DisplayMode>* _aidl_return) override;
    ndk::ScopedAStatus getCurrentDisplayMode(DisplayMode* _aidl_return) override;
    ndk::ScopedAStatus getDefaultDisplayMode(DisplayMode* _aidl_return) override;
    ndk::ScopedAStatus setDisplayMode(int32_t modeID, bool makeDefault) override;

  private:
    void initialize();

    static const std::map<int32_t, std::string> kModeMap;
    static const std::map<std::string, std::map<std::string, std::map<std::string, std::string>>> kDataMap;
    int32_t mCurrentModeId;
    int32_t mDefaultModeId;
};

}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
