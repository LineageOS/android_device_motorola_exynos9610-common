/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "DisplayModesService"

#include "DisplayModes.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <fstream>

using ::android::base::ReadFileToString;
using ::android::base::StringPrintf;
using ::android::base::Trim;

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {

static constexpr const char* kDefaultPath = "/data/vendor/display/.displaymodedefault";

#define COLOR_MODE_SYSFS_PATH "/sys/class/dqe/dqe/aosp_colors"
#define PANEL_SUPPLIER_SYSFS_PATH "/sys/class/panel/panel/panel_supplier"
#define TUNE_MODE1_SYSFS_PATH "/sys/class/dqe/dqe/tune_mode1"
#define TUNE_MODE2_SYSFS_PATH "/sys/class/dqe/dqe/tune_mode2"
#define TUNE_ONOFF_SYSFS_PATH "/sys/class/dqe/dqe/tune_onoff"

const std::map<int32_t, std::string> DisplayModes::kModeMap = {
    // clang-format off
    {0, "Natural"},
    {1, "Boosted"},
    {2, "Saturated"},
/*
    {90, "mode0 (bypass)"}, // seems to be default/off (visually the same as Saturated)
    {91, "mode1 (sRGB)"},   // same as Natural
    {92, "mode2 (Native)"}, // same as Boosted
    {93, "onoff (DCI-P3)"}, // same as Saturated
*/
    // clang-format on
};

// clang-format off
const std::map<std::string, std::map<std::string, std::map<std::string, std::string>>> DisplayModes::kDataMap = {
    {
        "hix83112a", { // from /vendor/etc/dqe/hixmax-hix83112a.xml
            {
                "mode1", {
                    { "cgc", "255,58,33,117,255,76,0,20,255,104,255,243,255,61,231,255,244,72,255,255,255,0,0,0,0,0,255,58,33,117,255,76,0,20,255,104,255,243,255,61,231,255,244,72,255,255,255,0,0,0" },
                    { "gamma", "0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254" },
                    { "hsc", "1,32,0,-20,-12,0,0,1,15,1,0,1,0,0,0,0,0,0,0,5,-10,0,10,30,170,230,240,155,70,32,0,10,64,51,204" }
                }
            },
            {
                "mode2", {
                    { "cgc", "255,20,0,0,255,20,0,20,255,0,255,223,255,20,222,251,255,0,255,255,255,0,0,0,0,0,255,20,0,0,255,20,0,20,255,0,255,223,255,20,222,251,255,0,255,255,255,0,0,0" },
                    { "gamma", "0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254" },
                    { "hsc", "1,56,0,0,0,0,0,1,15,1,-52,1,0,0,0,0,0,0,0,5,-10,0,10,30,170,230,240,155,70,32,0,10,64,51,204" }
                }
            },
            {
                "onoff", {
                    { "cgc", "255,0,0,0,255,0,0,0,255,0,255,254,255,20,245,252,255,0,255,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,255,254,255,20,245,252,255,0,255,255,255,0,0,0" },
                    { "gamma", "0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254" },
                    { "hsc", "1,0,0,0,0,0,0,1,15,1,0,1,0,0,0,0,0,0,0,5,-10,0,10,30,170,230,240,155,70,32,0,10,64,51,204" }
                },
            },
        }
    },
    {
        "nov36672a", { // from /vendor/etc/dqe/novatek-nov36672a.xml
            {
                "mode1", {
                    { "cgc", "255,61,33,119,255,74,0,28,255,104,255,239,255,65,228,255,244,72,254,255,253,0,0,0,0,0,255,61,33,119,255,74,0,28,255,104,255,239,255,65,228,255,244,72,254,255,253,0,0,0" },
                    { "gamma", "0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254" },
                    { "hsc", "1,54,0,0,0,0,0,1,15,1,0,1,0,0,0,0,0,0,0,5,-10,0,10,30,170,230,240,155,70,32,0,10,64,51,204" }
                }
            },
            {
                "mode2", {
                    { "cgc", "255,0,0,0,255,0,0,0,255,0,255,254,255,20,245,252,255,0,255,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,255,254,255,20,245,252,255,0,255,255,255,0,0,0" },
                    { "gamma", "0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254" },
                    { "hsc", "1,26,0,0,0,0,0,1,15,1,-43,1,1,0,0,0,0,0,-12,5,-10,0,10,30,170,230,240,155,70,32,0,10,64,51,204" }
                }
            },
            {
                "onoff", {
                    { "cgc", "255,0,0,0,255,0,0,0,255,0,255,254,255,20,245,252,255,0,255,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,255,254,255,20,245,252,255,0,255,255,255,0,0,0" },
                    { "gamma", "0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254,0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,254" },
                    { "hsc", "1,56,0,56,0,0,0,1,15,1,29,1,1,0,-12,0,0,0,-11,5,-10,0,10,30,170,230,240,155,70,32,0,10,64,51,204" }
                },
            },
        }
    }
};
// clang-format on

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value << std::endl;
}

template <typename T>
static T get(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;

    file >> result;
    return file.fail() ? def : result;
}

DisplayModes::DisplayModes() : mCurrentModeId(0), mDefaultModeId(0) {
    initialize();
    std::ifstream defaultFile(kDefaultPath);
    int value;

    defaultFile >> value;
    LOG(DEBUG) << "Default file read result " << value << " fail " << defaultFile.fail();
    if (defaultFile.fail()) {
        return;
    }

    for (const auto& entry : kModeMap) {
        if (value == entry.first) {
            mDefaultModeId = entry.first;
            break;
        }
    }

    setDisplayMode(mDefaultModeId, false);
}

// Methods from ::aidl::vendor::lineage::livedisplay::BnDisplayModes follow.
ndk::ScopedAStatus DisplayModes::getDisplayModes(std::vector<DisplayMode>* _aidl_return) {
    std::vector<DisplayMode> modes;

    for (const auto& entry : kModeMap) {
        if (entry.first < 3) modes.push_back({entry.first, entry.second});
    }

    *_aidl_return = modes;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus DisplayModes::getCurrentDisplayMode(DisplayMode* _aidl_return) {
    // decon_dqe_aosp_color_show always returns 0 so we have to keep track ourselves

    *_aidl_return = DisplayMode{mCurrentModeId, kModeMap.at(mCurrentModeId)};

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus DisplayModes::getDefaultDisplayMode(DisplayMode* _aidl_return) {
    *_aidl_return = DisplayMode{mDefaultModeId, kModeMap.at(mDefaultModeId)};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus DisplayModes::setDisplayMode(int32_t modeID, bool makeDefault) {
    const auto iter = kModeMap.find(modeID);
    if (iter == kModeMap.end()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    std::ofstream modeFile(COLOR_MODE_SYSFS_PATH);
    modeFile << iter->first;
    if (modeFile.fail()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    if (makeDefault) {
        std::ofstream defaultFile(kDefaultPath);
        defaultFile << iter->first;
        if (defaultFile.fail()) {
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
        }
        mDefaultModeId = iter->first;
    }

    mCurrentModeId = iter->first;

    return ndk::ScopedAStatus::ok();
}

void DisplayModes::initialize() {
    std::string panel = get(PANEL_SUPPLIER_SYSFS_PATH, std::string(""));

    if (panel.empty())
        return;

    for (const auto& entry : kDataMap) {
        if (!panel.compare(std::string(entry.first))) {
            auto data =  entry.second;
            auto mode1 = data["mode1"];
            set(TUNE_MODE1_SYSFS_PATH, StringPrintf("%s,%s,%s,", mode1["cgc"].c_str(), mode1["gamma"].c_str(), mode1["hsc"].c_str()));

            auto mode2 = data["mode2"];
            set(TUNE_MODE2_SYSFS_PATH, StringPrintf("%s,%s,%s,", mode2["cgc"].c_str(), mode2["gamma"].c_str(), mode2["hsc"].c_str()));

            auto onoff = data["onoff"];
            set(TUNE_ONOFF_SYSFS_PATH, StringPrintf("%s,%s,%s,", onoff["cgc"].c_str(), onoff["gamma"].c_str(), onoff["hsc"].c_str()));
            break;
        }
    }
}

}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
