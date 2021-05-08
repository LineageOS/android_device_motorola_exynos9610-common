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

#define LOG_TAG "vendor.lineage.livedisplay@2.0-service.exynos9610"

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>

#include "AdaptiveBacklight.h"
#include "DisplayModes.h"
#include "SunlightEnhancement.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::OK;
using android::sp;
using android::status_t;

using vendor::lineage::livedisplay::V2_0::IAdaptiveBacklight;
using vendor::lineage::livedisplay::V2_0::IDisplayModes;
using vendor::lineage::livedisplay::V2_0::implementation::AdaptiveBacklight;
using vendor::lineage::livedisplay::V2_0::implementation::DisplayModes;
using vendor::lineage::livedisplay::V2_0::implementation::SunlightEnhancement;
using vendor::lineage::livedisplay::V2_0::ISunlightEnhancement;

int main() {
    sp<IAdaptiveBacklight> adaptiveBacklight;
    sp<IDisplayModes> displayModes;
    sp<ISunlightEnhancement> sunlightEnhancement;
    status_t status;

    LOG(INFO) << "LiveDisplay HAL service is starting.";

    adaptiveBacklight = new AdaptiveBacklight();
    if (adaptiveBacklight == nullptr) {
        LOG(ERROR)
            << "Can not create an instance of LiveDisplay HAL AdaptiveBacklight Iface, exiting.";
        goto shutdown;
    }

    displayModes = new DisplayModes();
    if (displayModes == nullptr) {
        LOG(ERROR) << "Can not create an instance of LiveDisplay HAL DisplayModes Iface, exiting.";
        goto shutdown;
    }

    sunlightEnhancement = new SunlightEnhancement();
    if (sunlightEnhancement == nullptr) {
        LOG(ERROR)
            << "Can not create an instance of LiveDisplay HAL SunlightEnhancement Iface, exiting.";
        goto shutdown;
    }

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    status = adaptiveBacklight->registerAsService();
    if (status != OK) {
        LOG(ERROR) << "Could not register service for LiveDisplay HAL AdaptiveBacklight Iface ("
                   << status << ")";
        goto shutdown;
    }

    status = displayModes->registerAsService();
    if (status != OK) {
        LOG(ERROR) << "Could not register service for LiveDisplay HAL DisplayModes Iface ("
                   << status << ")";
        goto shutdown;
    }

    status = sunlightEnhancement->registerAsService();
    if (status != OK) {
        LOG(ERROR) << "Could not register service for LiveDisplay HAL SunlightEnhancement Iface ("
                   << status << ")";
        goto shutdown;
    }

    LOG(INFO) << "LiveDisplay HAL service is ready.";
    joinRpcThreadpool();
    // Should not pass this line

shutdown:
    // In normal operation, we don't expect the thread pool to shutdown
    LOG(ERROR) << "LiveDisplay HAL service is shutting down.";
    return 1;
}
