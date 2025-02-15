/*
 * Copyright (C) 2017 The Android Open Source Project
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
#define LOG_TAG "android.hardware.biometrics.fingerprint@2.3-service.mh2lm"
#define LOG_VERBOSE "android.hardware.biometrics.fingerprint@2.3-service.mh2lm"

#include <hardware/hw_auth_token.h>

#include <android-base/file.h>
#include <android-base/strings.h>
#include <hardware/fingerprint.h>
#include <hardware/hardware.h>
#include "BiometricsFingerprint.h"

#include <dlfcn.h>
#include <inttypes.h>
#include <unistd.h>

#include <cstdlib>

namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_3 {
namespace implementation {

// Supported fingerprint HAL version
static const uint16_t kVersion = HARDWARE_MODULE_API_VERSION(2, 1);

using ::android::base::StartsWith;

BiometricsFingerprint* BiometricsFingerprint::sInstance = nullptr;

BiometricsFingerprint::BiometricsFingerprint() : mClientCallback(nullptr), mDevice(nullptr) {
    sInstance = this;  // keep track of the most recent instance
    mDevice = openHal();
    if (!mDevice) {
        ALOGE("Can't open HAL module");
    }
}

BiometricsFingerprint::~BiometricsFingerprint() {
    ALOGV("~BiometricsFingerprint()");
    if (mDevice == nullptr) {
        ALOGE("No valid device");
        return;
    }
    int err;
    if (0 != (err = mDevice->rbs_uninitialize())) {
        ALOGE("Can't close fingerprint module, error: %d", err);
        return;
    }
    free(mDevice);
}

void BiometricsFingerprint::onErrorCallback(FingerprintError error, uint32_t vendorCode) {
    uint64_t devId = reinterpret_cast<uint64_t>(mDevice);
    ALOGD("onError(%d)", error);
    if (!mClientCallback->onError(devId, error, vendorCode).isOk()) {
        ALOGE("failed to invoke fingerprint onError callback");
    }
}

void BiometricsFingerprint::onEnumerateCallback(uint32_t fid, uint32_t gid,
                                                uint32_t samples_remaining) {
    uint64_t devId = reinterpret_cast<uint64_t>(mDevice);
    ALOGD("onEnumerate(fid=%d, gid=%d, rem=%d)", fid, gid, samples_remaining);
    if (!mClientCallback->onEnumerate(devId, fid, gid, samples_remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onEnumerate callback");
    }
}

void BiometricsFingerprint::onRemovedCallback(uint32_t fid, uint32_t gid,
                                              uint32_t samples_remaining) {
    uint64_t devId = reinterpret_cast<uint64_t>(mDevice);
    ALOGD("onRemoved(fid=%d, gid=%d, rem=%d)", fid, gid, samples_remaining);
    if (!mClientCallback->onRemoved(devId, fid, gid, samples_remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onRemoved callback");
    }
}

Return<RequestStatus> BiometricsFingerprint::ErrorFilter(int32_t error) {
    switch (error) {
        case 0:
            return RequestStatus::SYS_OK;
        case -2:
            return RequestStatus::SYS_ENOENT;
        case -4:
            return RequestStatus::SYS_EINTR;
        case -5:
            return RequestStatus::SYS_EIO;
        case -11:
            return RequestStatus::SYS_EAGAIN;
        case -12:
            return RequestStatus::SYS_ENOMEM;
        case -13:
            return RequestStatus::SYS_EACCES;
        case -14:
            return RequestStatus::SYS_EFAULT;
        case -16:
            return RequestStatus::SYS_EBUSY;
        case -22:
            return RequestStatus::SYS_EINVAL;
        case -28:
            return RequestStatus::SYS_ENOSPC;
        case -110:
            return RequestStatus::SYS_ETIMEDOUT;
        default:
            ALOGE("An unknown error returned from fingerprint vendor library: %d", error);
            return RequestStatus::SYS_UNKNOWN;
    }
}

Return<uint64_t> BiometricsFingerprint::setNotify(
        const sp<IBiometricsFingerprintClientCallback>& clientCallback) {
    std::lock_guard<std::mutex> lock(mClientCallbackMutex);
    mClientCallback = clientCallback;
    // This is here because HAL 2.1 doesn't have a way to propagate a
    // unique token for its driver. Subsequent versions should send a unique
    // token for each call to setNotify(). This is fine as long as there's only
    // one fingerprint device on the platform.
    return reinterpret_cast<uint64_t>(mDevice);
}

Return<uint64_t> BiometricsFingerprint::preEnroll() {
    mChallenge = (rand()|rand()<<20);
    return mChallenge;
}

Return<RequestStatus> BiometricsFingerprint::enroll(const hidl_array<uint8_t, 69>& hat,
                                                    uint32_t gid, uint32_t timeoutSec __unused) {
    int rc = 0;
    const hw_auth_token_t* authToken = reinterpret_cast<const hw_auth_token_t*>(hat.data());
    uint64_t devId = reinterpret_cast<uint64_t>(mDevice);

    if (authToken == NULL || authToken->timestamp == 0) {
        ALOGE("HAT is null");
        return RequestStatus::SYS_ENOENT;
    }

    if (authToken->challenge != mChallenge) {
        ALOGE("Challenge does not match");
        // Stock does a notify callback for this, but that doesn't seem necessary?
        return RequestStatus::SYS_EINVAL;
    }

    if (authToken->version != 0) {
        ALOGE("Invalid HAT version = %d", authToken->version);
        // Stock returns EPROTONOSUPPORT, but that isn't supported in HIDL
        return RequestStatus::SYS_EINVAL;
    }

    if ((authToken->challenge != mChallenge) &&
        !(authToken->authenticator_type & HW_AUTH_FINGERPRINT)) {
        ALOGE("Invalid authenticator type");
        // Stock returns EPERM, but that isn't supported in HIDL
        return RequestStatus::SYS_EINVAL;
    }

    rc = mDevice->rbs_chk_auth_token(authToken, sizeof(hw_auth_token_t));
    if (rc != 0) {
        ALOGE("Auth token check failed, error %d", rc);
        return ErrorFilter(rc);
    }

    rc = mDevice->rbs_chk_secure_id(gid, authToken->user_id);
    if (rc != 0) {
        ALOGD("Secure ID check failed, error %d", rc);
        if (rc != 0x21) {
            // Stock returns EPERM, but that isn't supported in HIDL
            return RequestStatus::SYS_EINVAL;
        }
        rc = mDevice->rbs_remove_fingerprint(gid, 0);
        if (rc == 0) {
            // After nuking everything, check if Secure ID is okay again
            rc = mDevice->rbs_chk_secure_id(gid, authToken->user_id);
            if (rc == 0) {
                ALOGD("Removed all fingerprints and secure ID check OK");
                goto continue_enroll;
            } else {
                ALOGD("Secure ID check failed, error %d", rc);
                // Stock returns EPERM, but that isn't supported in HIDL
                return ErrorFilter(rc);
            }
        } else {
            ALOGE("Remove all fingerprints failed, error %d", rc);
            // Stock returns EPERM, but that isn't supported in HIDL
            return ErrorFilter(rc);
        }
    }

continue_enroll:
    int seed = 0;
    int pre_enroll_rc = 0;
    do {
        seed = rand();
        pre_enroll_rc = mDevice->rbs_pre_enroll(gid, seed);
        if (pre_enroll_rc == 0) {
            rc = mDevice->rbs_enroll();
            if (rc == 0) return RequestStatus::SYS_OK;
            onErrorCallback(FingerprintError::ERROR_CANCELED, 0);
            if (rc != 4) return RequestStatus::SYS_OK;
            onErrorCallback(FingerprintError::ERROR_UNABLE_TO_PROCESS, 0);
            return RequestStatus::SYS_OK;
        }
    } while (pre_enroll_rc != 11);
    ALOGE("Pre-enroll failed, no space");

    onErrorCallback(FingerprintError::ERROR_CANCELED, 0);

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::postEnroll() {
    mChallenge = 0;
    return RequestStatus::SYS_OK;
}

Return<uint64_t> BiometricsFingerprint::getAuthenticatorId() {
    uint64_t authenticatorId;
    mDevice->rbs_get_authenticator_id(&authenticatorId);
    return authenticatorId;
}

Return<RequestStatus> BiometricsFingerprint::cancel() {
    if (hbmFodEnabled) {
        BiometricsFingerprint::onFingerUp();
    }
    return ErrorFilter(mDevice->rbs_cancel());
}

Return<RequestStatus> BiometricsFingerprint::enumerate() {
    int rc = 0;
    uint32_t num_fids = 0;
    uint32_t fids[5] = {};
    uint64_t devId = reinterpret_cast<uint64_t>(mDevice);
    if (mGid == 9999) {
        ALOGE("User ID empty");
        return RequestStatus::SYS_EINVAL;
    }

    rc = mDevice->rbs_get_fingerprint_ids(mGid, fids, &num_fids);
    if (rc != 0) {
        ALOGE("Enumerate failed, error %d", rc);
        return ErrorFilter(rc);
    }

    if (num_fids == 0) {
        onEnumerateCallback(0, mGid, 0);
    } else {
        for (int i = 0; i < num_fids; i++) {
            uint32_t samples_remaining = num_fids - i - 1;
            onEnumerateCallback(fids[i], mGid, samples_remaining);
        }
    }

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::remove(uint32_t gid, uint32_t fid) {
    int rc = 0;
    uint32_t num_fids = 0;
    uint32_t fids[5] = {};
    uint64_t devId = reinterpret_cast<uint64_t>(mDevice);

    // Get a list of fingerprints
    rc = mDevice->rbs_get_fingerprint_ids(gid, fids, &num_fids);
    if (rc != 0) {
        onErrorCallback(FingerprintError::ERROR_UNABLE_TO_REMOVE, 0);
        return RequestStatus::SYS_OK;
    }

    // Check if there even are fingerprints to remove
    if (num_fids == 0) {
        ALOGD("No fingerprints registered");
        onRemovedCallback(0, gid, 0);
        return RequestStatus::SYS_OK;
    }

    /*
        Remove the fingerprint.
        It seems that the RBS API supports deleting all fingerprints if the FID is 0, so we don't
        need to manually loop here.
        We do need to manually loop the callbacks, however.
    */
    rc = mDevice->rbs_remove_fingerprint(gid, fid);
    if (rc == 0) {
        if (fid != 0) {
            rc = mDevice->rbs_get_fingerprint_ids(gid, fids, &num_fids);
            if (!rc) {
                onErrorCallback(FingerprintError::ERROR_UNABLE_TO_REMOVE, 0);
            }
        } else {
            // Notify framework that all fingerprints are gone
            for (int i = 0; i < num_fids; i++) {
                uint32_t samples_remaining = num_fids - i - 1;
                onRemovedCallback(fids[i], gid, samples_remaining);
            }

            // Stock is also sending a cancel operation for some reason? why?
            onErrorCallback(FingerprintError::ERROR_CANCELED, 0);
            return RequestStatus::SYS_OK;
        }
    }

    /*
        Interestingly on stock, removal can fail, but it will send a notify call that the
        fingerprint was removed anyway.
        The only explanation for this is that instead of having a dangling fingerprint where it may
        or may not have been successfully removed, it instructs the framework to invalidate the
        fingerprint anyway, just in case.
    */
    onRemovedCallback(fid, gid, 0);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::setActiveGroup(uint32_t gid,
                                                            const hidl_string& storePath) {
    mGid = gid;
    if (storePath.size() >= PATH_MAX || storePath.size() <= 0) {
        ALOGE("Bad path length: %zd", storePath.size());
        return RequestStatus::SYS_EINVAL;
    }
    std::string mutableStorePath = storePath;
    if (StartsWith(mutableStorePath, "/data/system/users/")) {
        mutableStorePath = "/data/vendor_de/";
        mutableStorePath +=
                static_cast<std::string>(storePath).substr(strlen("/data/system/users/"));
    }
    if (access(mutableStorePath.c_str(), W_OK)) {
        return RequestStatus::SYS_EINVAL;
    }

    return ErrorFilter(mDevice->rbs_active_user_group(gid, mutableStorePath.c_str()));
}

Return<RequestStatus> BiometricsFingerprint::authenticate(uint64_t operationId, uint32_t gid) {
    int rc = 0;
    uint64_t devId = reinterpret_cast<uint64_t>(mDevice);
    mOperationId = operationId;
    mGid = gid;

    rc = mDevice->rbs_authenticator(gid, 0, 0, operationId);
    if (rc != 0) {
        onErrorCallback(FingerprintError::ERROR_CANCELED, 0);
        if (rc == 4) {
            onErrorCallback(FingerprintError::ERROR_HW_UNAVAILABLE, 0);
        }
    }
    return ErrorFilter(rc);
}

IBiometricsFingerprint* BiometricsFingerprint::getInstance() {
    if (!sInstance) {
        sInstance = new BiometricsFingerprint();
    }
    return sInstance;
}

rbs_fingerprint_device_t* BiometricsFingerprint::openHal() {
    int err;
    void* rbs_handle;
    uint8_t masterkey[0x100];

    ALOGD("Opening fingerprint hal library...");
    rbs_handle = dlopen("libRbsFlow.so", RTLD_NOW);

    if (rbs_handle == nullptr) {
        ALOGE("No valid fingerprint module");
        return nullptr;
    }

    rbs_fingerprint_device_t* fp_device =
            (rbs_fingerprint_device_t*)malloc(sizeof(rbs_fingerprint_device_t));

    fp_device->rbs_initialize = reinterpret_cast<typeof(fp_device->rbs_initialize)>(
            dlsym(rbs_handle, "rbs_initialize"));

    fp_device->rbs_uninitialize = reinterpret_cast<typeof(fp_device->rbs_uninitialize)>(
            dlsym(rbs_handle, "rbs_uninitialize"));

    fp_device->rbs_cancel =
            reinterpret_cast<typeof(fp_device->rbs_cancel)>(dlsym(rbs_handle, "rbs_cancel"));

    fp_device->rbs_active_user_group = reinterpret_cast<typeof(fp_device->rbs_active_user_group)>(
            dlsym(rbs_handle, "rbs_active_user_group"));

    fp_device->rbs_set_data_path = reinterpret_cast<typeof(fp_device->rbs_set_data_path)>(
            dlsym(rbs_handle, "rbs_set_data_path"));

    fp_device->rbs_chk_secure_id = reinterpret_cast<typeof(fp_device->rbs_chk_secure_id)>(
            dlsym(rbs_handle, "rbs_chk_secure_id"));

    fp_device->rbs_pre_enroll = reinterpret_cast<typeof(fp_device->rbs_pre_enroll)>(
            dlsym(rbs_handle, "rbs_pre_enroll"));

    fp_device->rbs_enroll =
            reinterpret_cast<typeof(fp_device->rbs_enroll)>(dlsym(rbs_handle, "rbs_enroll"));

    fp_device->rbs_post_enroll = reinterpret_cast<typeof(fp_device->rbs_post_enroll)>(
            dlsym(rbs_handle, "rbs_post_enroll"));

    fp_device->rbs_chk_auth_token = reinterpret_cast<typeof(fp_device->rbs_chk_auth_token)>(
            dlsym(rbs_handle, "rbs_chk_auth_token"));

    fp_device->rbs_authenticator = reinterpret_cast<typeof(fp_device->rbs_authenticator)>(
            dlsym(rbs_handle, "rbs_authenticator"));

    fp_device->rbs_remove_fingerprint = reinterpret_cast<typeof(fp_device->rbs_remove_fingerprint)>(
            dlsym(rbs_handle, "rbs_remove_fingerprint"));

    fp_device->rbs_get_fingerprint_ids =
            reinterpret_cast<typeof(fp_device->rbs_get_fingerprint_ids)>(
                    dlsym(rbs_handle, "rbs_get_fingerprint_ids"));

    fp_device->rbs_get_authenticator_id =
            reinterpret_cast<typeof(fp_device->rbs_get_authenticator_id)>(
                    dlsym(rbs_handle, "rbs_get_authenticator_id"));

    fp_device->rbs_set_on_callback_proc =
            reinterpret_cast<typeof(fp_device->rbs_set_on_callback_proc)>(
                    dlsym(rbs_handle, "rbs_set_on_callback_proc"));

    fp_device->rbs_extra_api =
            reinterpret_cast<typeof(fp_device->rbs_extra_api)>(dlsym(rbs_handle, "rbs_extra_api"));

    fp_device->rbs_set_on_callback_proc((void*)BiometricsFingerprint::notify);

    if ((err = fp_device->rbs_initialize(0, 0)) != 0) {
        ALOGE("Can't open fingerprint, error %d", err);
        free(fp_device);
        return nullptr;
    }

    return fp_device;
}

void BiometricsFingerprint::notify(uint32_t eventId, uint32_t value1, uint32_t value2, void* buffer,
                                   uint32_t /* buffer_size */) {
    BiometricsFingerprint* thisPtr =
            static_cast<BiometricsFingerprint*>(BiometricsFingerprint::getInstance());
    std::lock_guard<std::mutex> lock(thisPtr->mClientCallbackMutex);
    if (thisPtr == nullptr || thisPtr->mClientCallback == nullptr) {
        ALOGE("Receiving callbacks before the client callback is registered.");
        return;
    }
    const uint64_t devId = reinterpret_cast<uint64_t>(thisPtr->mDevice);
    switch (eventId) {
        // Error
        case 0x3eb:
        case 0x401: {
            ALOGD("onError(%d)", FingerprintError::ERROR_CANCELED);
            if (!thisPtr->mClientCallback->onError(devId, FingerprintError::ERROR_CANCELED, 0)
                         .isOk()) {
                ALOGE("failed to invoke fingerprint onError callback");
            }
        } break;
        case 0x40e: {
            ALOGD("onError(%d)", FingerprintError::ERROR_TIMEOUT);
            if (!thisPtr->mClientCallback->onError(devId, FingerprintError::ERROR_TIMEOUT, 0)
                         .isOk()) {
                ALOGE("failed to invoke fingerprint onError callback");
            }
        } break;
        // Acquired
        case 0x3ec:
        case 0x3ed: {
            ALOGD("onAcquired(%d)", FingerprintAcquiredInfo::ACQUIRED_TOO_SLOW);
            if (!thisPtr->mClientCallback
                         ->onAcquired(devId, FingerprintAcquiredInfo::ACQUIRED_TOO_SLOW, 0)
                         .isOk()) {
                ALOGE("failed to invoke fingerprint onAcquired callback");
            }
        } break;
        case 0x3ee:
        case 0x3ef: {
            ALOGD("onAcquired(%d)", FingerprintAcquiredInfo::ACQUIRED_VENDOR);
            if (!thisPtr->mClientCallback
                         ->onAcquired(devId, FingerprintAcquiredInfo::ACQUIRED_VENDOR, 0)
                         .isOk()) {
                ALOGE("failed to invoke fingerprint onAcquired callback");
            }
        } break;
        case 0x3f5: {
            ALOGD("onAcquired(%d)", FingerprintAcquiredInfo::ACQUIRED_INSUFFICIENT);
            if (!thisPtr->mClientCallback
                         ->onAcquired(devId, FingerprintAcquiredInfo::ACQUIRED_INSUFFICIENT, 0)
                         .isOk()) {
                ALOGE("failed to invoke fingerprint onAcquired callback");
            }
        } break;
        case 0x3f7:
        case 0x3f8: {
            ALOGD("onAcquired(%d)", FingerprintAcquiredInfo::ACQUIRED_PARTIAL);
            if (!thisPtr->mClientCallback
                         ->onAcquired(devId, FingerprintAcquiredInfo::ACQUIRED_PARTIAL, 0)
                         .isOk()) {
                ALOGE("failed to invoke fingerprint onAcquired callback");
            }
        } break;
        case 0x3f9:
        case 0x3fa:
        case 0x3fb: {
            ALOGD("onAcquired(%d)", FingerprintAcquiredInfo::ACQUIRED_TOO_FAST);
            if (!thisPtr->mClientCallback
                         ->onAcquired(devId, FingerprintAcquiredInfo::ACQUIRED_TOO_FAST, 0)
                         .isOk()) {
                ALOGE("failed to invoke fingerprint onAcquired callback");
            }
        } break;
        case 0x3fe: {
            ALOGD("onAcquired(%d)", FingerprintAcquiredInfo::ACQUIRED_GOOD);
            if (!thisPtr->mClientCallback
                         ->onAcquired(devId, FingerprintAcquiredInfo::ACQUIRED_GOOD, 0)
                         .isOk()) {
                ALOGE("failed to invoke fingerprint onAcquired callback");
            }
        } break;
        // Enrolling
        case 0x40d: {
            uint32_t fid = value1;
            uint32_t samples_remaining = value2;
            ALOGD("onEnrollResult(fid=%d, gid=%d, rem=%d)", fid, thisPtr->mGid, samples_remaining);
            if (!thisPtr->mClientCallback
                         ->onEnrollResult(devId, fid, thisPtr->mGid, samples_remaining)
                         .isOk()) {
                ALOGE("failed to invoke fingerprint onEnrollResult callback");
            }
        } break;
        // Authenticated
        case 0x3f2:
        case 0x3f3: {
            uint32_t gid = value1;
            uint32_t fid = value2;
            if (fid != 0) {
                ALOGD("onAuthenticated(fid=%d, gid=%d)", fid, gid);
                const uint8_t* hat = reinterpret_cast<const uint8_t*>(buffer);
                const hidl_vec<uint8_t> token(
                        std::vector<uint8_t>(hat, hat + sizeof(hw_auth_token_t)));
                if (!thisPtr->mClientCallback->onAuthenticated(devId, fid, gid, token).isOk()) {
                    ALOGE("failed to invoke fingerprint onAuthenticated callback");
                }
            } else {
                // Not a recognized fingerprint
                if (!thisPtr->mClientCallback->onAuthenticated(devId, fid, gid, hidl_vec<uint8_t>())
                             .isOk()) {
                    ALOGE("failed to invoke fingerprint onAuthenticated callback");
                }
            }
            break;
        }
    }
    // Disable FOD
    if (thisPtr->hbmFodEnabled) {
        thisPtr->onFingerUp();
    }
}

#define FOD_HBM_PATH "/sys/devices/virtual/panel/brightness/fp_lhbm"

void BiometricsFingerprint::setFodHbm(bool status) {
    android::base::WriteStringToFile(status ? "1" : "0", FOD_HBM_PATH);
}

// ::V2_3::IBiometricsFingerprint follow.

Return<bool> BiometricsFingerprint::isUdfps(uint32_t) {
    return false;
}

Return<void> BiometricsFingerprint::onFingerDown(uint32_t, uint32_t, float, float) {
    std::lock_guard<std::mutex> lock(mSetHbmFodMutex);
    uint32_t param = 0x65;
    uint32_t rbs_param[8] = {};
    uint32_t rbs_param_size = sizeof(rbs_param);

    if (hbmFodEnabled) return Void();

    mDevice->rbs_extra_api(7, &param, 4, rbs_param, &rbs_param_size);

    setFodHbm(true);

    hbmFodEnabled = true;

    return Void();
}

Return<void> BiometricsFingerprint::onFingerUp() {
    std::lock_guard<std::mutex> lock(mSetHbmFodMutex);
    uint32_t param = 0x66;
    uint32_t rbs_param[8] = {};
    uint32_t rbs_param_size = sizeof(rbs_param);

    if (!hbmFodEnabled) return Void();

    mDevice->rbs_extra_api(7, &param, 4, rbs_param, &rbs_param_size);

    setFodHbm(false);

    hbmFodEnabled = false;

    return Void();
}

}  // namespace implementation
}  // namespace V2_3
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
