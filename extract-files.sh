#!/bin/bash
#
# SPDX-FileCopyrightText: 2016 The CyanogenMod Project
# SPDX-FileCopyrightText: 2017-2024 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

export TARGET_ENABLE_CHECKELF=false

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

ONLY_COMMON=
ONLY_FIRMWARE=
ONLY_TARGET=
KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        --only-common )
                ONLY_COMMON=true
                ;;
        --only-firmware )
                ONLY_FIRMWARE=true
                ;;
        --only-target )
                ONLY_TARGET=true
                ;;
        -n | --no-cleanup )
                CLEAN_VENDOR=false
                ;;
        -k | --kang )
                KANG="--kang"
                ;;
        -s | --section )
                SECTION="${2}"
                shift
                CLEAN_VENDOR=false
                ;;
        * )
                SRC="${1}"
                ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

function blob_fixup() {
    case "${1}" in
        # libmedia symbols moved
        lib64/libmediaadaptor.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --replace-needed "libmedia.so" "libmedia_ims.so" "${2}"
            ;;
        # libnetutils shim
        vendor/bin/wfc-pkt-router)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --replace-needed "libnetutils.so" "libip_checksum_shim.so" "${2}"
            ;;
        # Missing libutils symbols
        vendor/lib*/sensors.rp.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --replace-needed "libutils.so" "libutils-v32.so" "${2}"
            ;;
        vendor/lib/libaudioproxy.so)
            [ "$2" = "" ] && return 0
            for LIBAUDIOPROXY_SHIM in $(grep -L "libaudioproxy_shim.so" "${2}"); do
                "${PATCHELF}" --add-needed "libaudioproxy_shim.so" "${LIBAUDIOPROXY_SHIM}"
            done
            ;;
        vendor/bin/charge_only_mode)
            [ "$2" = "" ] && return 0
            for LIBMEMSET in $(grep -L "libmemset_shim.so" "${2}"); do
                "${PATCHELF}" --add-needed "libmemset_shim.so" "${LIBMEMSET}"
            done
            ;;
        vendor/lib*/libhifills.so)
            [ "$2" = "" ] && return 0
            for LIBDEMANGLE in $(grep -L "libdemangle.so" "${2}"); do
                "${PATCHELF}" --add-needed "libdemangle.so" "${LIBDEMANGLE}"
            done
            ;;
        vendor/lib64/libstrongswan.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --add-needed "libcrypto_shim.so" "${2}"
            ;;
        # Remove libhidltransport/libhwbinder references
        vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-rbs|vendor/lib64/hw/android.hardware.gnss@1.0-impl.samsung.so|vendor/lib64/hw/android.hardware.gnss@1.1-impl.samsung.so|vendor/lib64/hw/android.hardware.gnss@2.0-impl.samsung.so|vendor/lib64/hw/vendor.samsung.hardware.gnss@1.0-impl.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --remove-needed "libhidltransport.so" "${2}"
            "${PATCHELF}" --remove-needed "libhwbinder.so" "${2}"
            ;;
        # Remove libhwbinder references
        vendor/etc/permissions/com.motorola.motosignature.xml)
            [ "$2" = "" ] && return 0
            sed -i 's|/system/framework|/vendor/framework|' "${2}"
            ;;
        *)
            return 1
            ;;
    esac

    return 0
}

function blob_fixup_dry() {
    blob_fixup "$1" ""
}

if [ -z "${ONLY_FIRMWARE}" ] && [ -z "${ONLY_TARGET}" ]; then
    # Initialize the helper for common device
    setup_vendor "${DEVICE_COMMON}" "${VENDOR_COMMON:-$VENDOR}" "${ANDROID_ROOT}" true "${CLEAN_VENDOR}"

    extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
    extract "${MY_DIR}/proprietary-files-vendor.txt" "${SRC}" "${KANG}" --section "${SECTION}"

    # Reinitialize the helper for device
    source "${MY_DIR}/../../${VENDOR}/${DEVICE}/extract-files.sh"
    setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"
fi

if [ -z "${ONLY_COMMON}" ] && [ -s "${MY_DIR}/../../${VENDOR}/${DEVICE}/proprietary-files.txt" ]; then
    # Reinitialize the helper for device
    source "${MY_DIR}/../../${VENDOR}/${DEVICE}/extract-files.sh"
    setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

    if [ -z "${ONLY_FIRMWARE}" ]; then
        extract "${MY_DIR}/../../${VENDOR}/${DEVICE}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
    fi

    if [ -z "${SECTION}" ] && [ -f "${MY_DIR}/../../${VENDOR}/${DEVICE}/proprietary-firmware.txt" ]; then
        extract_firmware "${MY_DIR}/../../${VENDOR}/${DEVICE}/proprietary-firmware.txt" "${SRC}"
    fi
fi

"${MY_DIR}/setup-makefiles.sh"
