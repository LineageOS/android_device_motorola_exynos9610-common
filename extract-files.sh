#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

DEVICE_COMMON=exynos9610-common
VENDOR=motorola

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        -n | --no-cleanup )
                CLEAN_VENDOR=false
                ;;
        -k | --kang )
                KANG="--kang"
                ;;
        -s | --section )
                SECTION="${2}"; shift
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

# Initialize the helper
setup_vendor "${DEVICE_COMMON}" "${VENDOR}" "${ANDROID_ROOT}" true "${CLEAN_VENDOR}"

extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
extract "${MY_DIR}/proprietary-files-vendor.txt" "${SRC}" "${KANG}" --section "${SECTION}"

# Fix proprietary blobs
BLOB_ROOT="${ANDROID_ROOT}"/vendor/"${VENDOR}"/"${DEVICE_COMMON}"/proprietary

"${PATCHELF}" --replace-needed "libmedia.so" "libmedia_ims.so" "${BLOB_ROOT}"/lib64/libmediaadaptor.so

for LIBAUDIOPROXY_SHIM in $(grep -L "libaudioproxy_shim.so" "${BLOB_ROOT}"/vendor/lib/libaudioproxy.so); do
    "${PATCHELF}" --add-needed libaudioproxy_shim.so "${LIBAUDIOPROXY_SHIM}"
done
for LIBMEMSET in $(grep -L "libmemset.so" "${BLOB_ROOT}"/vendor/bin/charge_only_mode); do
    "${PATCHELF}" --add-needed "libmemset.so" "${LIBMEMSET}"
done
for LIBDEMANGLE in $(grep -L "libdemangle.so" "${BLOB_ROOT}"/vendor/lib/libhifills.so); do
    "${PATCHELF}" --add-needed "libdemangle.so" "${LIBDEMANGLE}"
done
for LIBDEMANGLE64 in $(grep -L "libdemangle.so" "${BLOB_ROOT}"/vendor/lib64/libhifills.so); do
    "${PATCHELF}" --add-needed "libdemangle.so" "${LIBDEMANGLE64}"
done

# Remove libhidltransport dependency
"${PATCHELF}" --remove-needed "libhidltransport.so" "${BLOB_ROOT}"/vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-rbs
"${PATCHELF}" --remove-needed "libhidltransport.so" "${BLOB_ROOT}"/vendor/lib64/hw/android.hardware.gnss@1.0-impl.samsung.so
"${PATCHELF}" --remove-needed "libhidltransport.so" "${BLOB_ROOT}"/vendor/lib64/hw/android.hardware.gnss@1.1-impl.samsung.so
"${PATCHELF}" --remove-needed "libhidltransport.so" "${BLOB_ROOT}"/vendor/lib64/hw/android.hardware.gnss@2.0-impl.samsung.so
"${PATCHELF}" --remove-needed "libhidltransport.so" "${BLOB_ROOT}"/vendor/lib64/hw/vendor.samsung.hardware.gnss@1.0-impl.so

# Remove libhwbinder dependency
"${PATCHELF}" --remove-needed "libhwbinder.so" "${BLOB_ROOT}"/vendor/lib64/hw/android.hardware.gnss@1.0-impl.samsung.so
"${PATCHELF}" --remove-needed "libhwbinder.so" "${BLOB_ROOT}"/vendor/lib64/hw/android.hardware.gnss@1.1-impl.samsung.so
"${PATCHELF}" --remove-needed "libhwbinder.so" "${BLOB_ROOT}"/vendor/lib64/hw/android.hardware.gnss@2.0-impl.samsung.so

"${MY_DIR}/setup-makefiles.sh"
