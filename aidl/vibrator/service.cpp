/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "android.hardware.vibrator-service.exynos9610"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "Vibrator.h"

using aidl::android::hardware::vibrator::Vibrator;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    LOG(INFO) << "Vibrator HAL service is starting.";

    std::shared_ptr<Vibrator> vibrator = ndk::SharedRefBase::make<Vibrator>();
    if (vibrator == nullptr) {
        LOG(ERROR) << "Can not create an instance of Vibrator HAL IVibrator, exiting.";
        goto shutdown;
    }

    {
        const std::string instance = std::string(Vibrator::descriptor) + "/default";
        if (AServiceManager_addService(vibrator->asBinder().get(), instance.c_str()) != STATUS_OK) {
            LOG(ERROR) << "Could not register service for Vibrator HAL";
            goto shutdown;
        }
    }

    LOG(INFO) << "Vibrator HAL service is Ready.";
    ABinderProcess_joinThreadPool();

shutdown:
    // In normal operation, we don't expect the thread pool to shutdown
    LOG(ERROR) << "Vibrator HAL failed to join thread pool.";
    return 1;
}
