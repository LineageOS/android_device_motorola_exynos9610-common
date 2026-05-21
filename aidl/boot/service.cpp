/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "aidl.android.hardware.boot-service.default"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>

#include "BootControl.h"

using aidl::android::hardware::boot::implementation::BootControl;
using aidl::android::hardware::boot::IBootControl;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;

int main(int, char* argv[]) {
    android::base::InitLogging(argv, android::base::KernelLogger);
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<IBootControl> service = ndk::SharedRefBase::make<BootControl>();

    const std::string instance = std::string(IBootControl::descriptor) + "/default";
    auto status = AServiceManager_addService(service->asBinder().get(), instance.c_str());
    CHECK_EQ(status, STATUS_OK) << "Failed to add service " << instance << " " << status;
    LOG(INFO) << "IBootControl AIDL service running...";

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
