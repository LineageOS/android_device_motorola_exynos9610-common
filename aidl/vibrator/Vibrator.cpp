/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "android.hardware.vibrator-service.exynos9610"

#include <log/log.h>

#include <android-base/stringprintf.h>

#include "Vibrator.h"

#include <cinttypes>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <thread>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value << std::endl;
}

Vibrator::Vibrator() : mCurrentAmplitude(1.0f) {}

static uint32_t amplitudeToBrightness(float amplitude) {
    return static_cast<uint32_t>(amplitude * 255.0f);
}

ndk::ScopedAStatus Vibrator::getCapabilities(int32_t* _aidl_return) {
    *_aidl_return = CAP_ON_CALLBACK | CAP_AMPLITUDE_CONTROL | CAP_PERFORM_CALLBACK | CAP_COMPOSITION;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::on(int32_t timeout_ms, const std::shared_ptr<IVibratorCallback>& callback) {
    set("/sys/class/leds/vibrator/brightness", amplitudeToBrightness(mCurrentAmplitude));
    set("/sys/class/leds/vibrator/state", 1);
    set("/sys/class/leds/vibrator/duration", timeout_ms);
    set("/sys/class/leds/vibrator/activate", 1);

    if (callback) {
        std::thread([callback, timeout_ms]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
            callback->onComplete();
        }).detach();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::off() {
    set("/sys/class/leds/vibrator/activate", 0);
    set("/sys/class/leds/vibrator/state", 0);

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setAmplitude(float amplitude) {
    if (amplitude < 0.0f || amplitude > 1.0f) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    mCurrentAmplitude = amplitude;
    set("/sys/class/leds/vibrator/brightness", amplitudeToBrightness(amplitude));
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setExternalControl(bool /*enabled*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getSupportedEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = {
        Effect::CLICK,
        Effect::DOUBLE_CLICK,
        Effect::TICK,
        Effect::THUD,
        Effect::POP,
        Effect::HEAVY_CLICK,
    };
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::perform(Effect effect, EffectStrength strength,
        const std::shared_ptr<IVibratorCallback>& callback, int32_t* _aidl_return) {
    int32_t duration_ms;
    switch (effect) {
        case Effect::CLICK:
            duration_ms = 100;
            break;
        case Effect::DOUBLE_CLICK:
            duration_ms = 200;
            break;
        case Effect::TICK:
            duration_ms = 100;
            break;
        case Effect::THUD:
            duration_ms = 150;
            break;
        case Effect::POP:
            duration_ms = 100;
            break;
        case Effect::HEAVY_CLICK:
            duration_ms = 120;
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    float amplitude;
    switch (strength) {
        case EffectStrength::LIGHT:
            amplitude = 0.4f;
            break;
        case EffectStrength::MEDIUM:
            amplitude = 0.7f;
            break;
        case EffectStrength::STRONG:
            amplitude = 1.0f;
            break;
        default:
            amplitude = 0.7f;
            break;
    }

    uint32_t intensity = amplitudeToBrightness(amplitude);
    int32_t ret_duration = duration_ms;

    if (effect == Effect::DOUBLE_CLICK) {
        set("/sys/class/leds/vibrator/brightness", intensity);
        set("/sys/class/leds/vibrator/duration", 100);
        set("/sys/class/leds/vibrator/state", 1);
        set("/sys/class/leds/vibrator/activate", 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        set("/sys/class/leds/vibrator/activate", 0);
        set("/sys/class/leds/vibrator/state", 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        set("/sys/class/leds/vibrator/brightness", intensity);
        set("/sys/class/leds/vibrator/duration", 100);
        set("/sys/class/leds/vibrator/state", 1);
        set("/sys/class/leds/vibrator/activate", 1);
    } else {
        set("/sys/class/leds/vibrator/brightness", intensity);
        set("/sys/class/leds/vibrator/duration", duration_ms);
        set("/sys/class/leds/vibrator/state", 1);
        set("/sys/class/leds/vibrator/activate", 1);
    }

    *_aidl_return = ret_duration;

    if (callback) {
        std::thread([callback, ret_duration]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(ret_duration));
            callback->onComplete();
        }).detach();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getCompositionDelayMax(int32_t* _aidl_return) {
    *_aidl_return = 5000;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getCompositionSizeMax(int32_t* _aidl_return) {
    *_aidl_return = 16;
    return ndk::ScopedAStatus::ok();
}

static int32_t getPrimitiveDurationMs(CompositePrimitive primitive) {
    switch (primitive) {
        case CompositePrimitive::CLICK:
            return 80;
        case CompositePrimitive::TICK:
            return 40;
        case CompositePrimitive::THUD:
            return 150;
        case CompositePrimitive::DOUBLE_CLICK:
            return 250;
        default:
            return 0;
    }
}

ndk::ScopedAStatus Vibrator::getSupportedPrimitives(std::vector<CompositePrimitive>* _aidl_return) {
    *_aidl_return = {
        CompositePrimitive::CLICK,
        CompositePrimitive::TICK,
        CompositePrimitive::THUD,
        CompositePrimitive::DOUBLE_CLICK,
    };
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getPrimitiveDuration(CompositePrimitive in_primitive, int32_t* _aidl_return) {
    int32_t duration = getPrimitiveDurationMs(in_primitive);
    if (duration == 0) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    *_aidl_return = duration;
    return ndk::ScopedAStatus::ok();
}

static void playPrimitive(CompositePrimitive primitive, float scale) {
    int32_t duration = getPrimitiveDurationMs(primitive);
    uint32_t brightness = static_cast<uint32_t>(scale * 255.0f);
    
    set("/sys/class/leds/vibrator/brightness", brightness);
    
    if (primitive == CompositePrimitive::DOUBLE_CLICK) {
        set("/sys/class/leds/vibrator/duration", 80);
        set("/sys/class/leds/vibrator/state", 1);
        set("/sys/class/leds/vibrator/activate", 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        set("/sys/class/leds/vibrator/activate", 0);
        set("/sys/class/leds/vibrator/state", 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        set("/sys/class/leds/vibrator/brightness", brightness);
        set("/sys/class/leds/vibrator/duration", 80);
        set("/sys/class/leds/vibrator/state", 1);
        set("/sys/class/leds/vibrator/activate", 1);
    } else {
        set("/sys/class/leds/vibrator/duration", duration);
        set("/sys/class/leds/vibrator/state", 1);
        set("/sys/class/leds/vibrator/activate", 1);
    }
}

ndk::ScopedAStatus Vibrator::compose(const std::vector<CompositeEffect>& in_composite,
        const std::shared_ptr<IVibratorCallback>& in_callback) {
    if (in_composite.empty()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    std::thread([in_composite, in_callback]() {
        for (size_t i = 0; i < in_composite.size(); i++) {
            int32_t dur = getPrimitiveDurationMs(in_composite[i].primitive);
            if (dur == 0) continue;

            playPrimitive(in_composite[i].primitive, in_composite[i].scale);
            std::this_thread::sleep_for(std::chrono::milliseconds(dur + 20));
            set("/sys/class/leds/vibrator/activate", 0);
            set("/sys/class/leds/vibrator/state", 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (in_callback) {
            in_callback->onComplete();
        }
    }).detach();

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getSupportedAlwaysOnEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = {};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::alwaysOnEnable(int32_t /*id*/, Effect /*effect*/, EffectStrength /*strength*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::alwaysOnDisable(int32_t /*id*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
