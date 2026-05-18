/*
 * Copyright (C) 2021 The LineageOS Project
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

#define LOG_TAG "vendor.lineage.livedisplay-service.exynos9610"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>

#include "AdaptiveBacklight.h"
#include "DisplayModes.h"
#include "SunlightEnhancement.h"

using ::aidl::vendor::lineage::livedisplay::AdaptiveBacklight;
using ::aidl::vendor::lineage::livedisplay::DisplayModes;
using ::aidl::vendor::lineage::livedisplay::SunlightEnhancement;

int main() {
    android::ProcessState::self()->setThreadPoolMaxThreadCount(1);
    android::ProcessState::self()->startThreadPool();

    std::shared_ptr<AdaptiveBacklight> adaptiveBacklight =
            ndk::SharedRefBase::make<AdaptiveBacklight>();
    std::shared_ptr<DisplayModes> displayModes = ndk::SharedRefBase::make<DisplayModes>();
    std::shared_ptr<SunlightEnhancement> sunlightEnhancement =
            ndk::SharedRefBase::make<SunlightEnhancement>();
    binder_status_t status;

    LOG(INFO) << "LiveDisplay HAL service is starting.";

    if (adaptiveBacklight == nullptr) {
        LOG(ERROR)
            << "Can not create an instance of LiveDisplay HAL AdaptiveBacklight Iface, exiting.";
        goto shutdown;
    }

    if (displayModes == nullptr) {
        LOG(ERROR) << "Can not create an instance of LiveDisplay HAL DisplayModes Iface, exiting.";
        goto shutdown;
    }

    if (sunlightEnhancement == nullptr) {
        LOG(ERROR)
            << "Can not create an instance of LiveDisplay HAL SunlightEnhancement Iface, exiting.";
        goto shutdown;
    }

    {
        std::string instance = std::string(AdaptiveBacklight::descriptor) + "/default";
        status = AServiceManager_addService(adaptiveBacklight->asBinder().get(), instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR) << "Could not register service for LiveDisplay HAL AdaptiveBacklight Iface ("
                       << status << ")";
            goto shutdown;
        }
    }

    {
        std::string instance = std::string(DisplayModes::descriptor) + "/default";
        status = AServiceManager_addService(displayModes->asBinder().get(), instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR) << "Could not register service for LiveDisplay HAL DisplayModes Iface ("
                       << status << ")";
            goto shutdown;
        }
    }

    {
        std::string instance = std::string(SunlightEnhancement::descriptor) + "/default";
        status =
                AServiceManager_addService(sunlightEnhancement->asBinder().get(), instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR)
                << "Could not register service for LiveDisplay HAL SunlightEnhancement Iface ("
                << status << ")";
            goto shutdown;
        }
    }

    LOG(INFO) << "LiveDisplay HAL service is ready.";
    ABinderProcess_joinThreadPool();
    // Should not pass this line

shutdown:
    // In normal operation, we don't expect the thread pool to shutdown
    LOG(ERROR) << "LiveDisplay HAL service is shutting down.";
    return 1;
}
