#!/bin/bash
#
# Copyright (C) 2017-2020 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

VENDOR=motorola
DEVICE_COMMON=exynos9610-common

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

LINEAGE_ROOT="${MY_DIR}"/../../..

HELPER="${LINEAGE_ROOT}/vendor/lineage/build/tools/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

SECTION=
KANG=

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
setup_vendor "${DEVICE_COMMON}" "${VENDOR}" "${LINEAGE_ROOT}" true "${CLEAN_VENDOR}"

extract "${MY_DIR}/proprietary-files.txt" "${SRC}" \
        "${KANG}" --section "${SECTION}"

extract "${MY_DIR}/proprietary-files-vendor.txt" "${SRC}" \
        "${KANG}" --section "${SECTION}"

# Fix proprietary blobs
BLOB_ROOT="$LINEAGE_ROOT"/vendor/"$VENDOR"/"$DEVICE_COMMON"/proprietary
patchelf --replace-needed libmedia.so libmedia_ims.so $BLOB_ROOT/lib64/libmediaadaptor.so
patchelf --add-needed libaudioproxy_shim.so $BLOB_ROOT/vendor/lib/libaudioproxy.so
patchelf --add-needed libmemset.so $BLOB_ROOT/vendor/bin/charge_only_mode

# Remove libhidltransport dependency
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-rbs
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/bin/hw/android.hardware.drm@1.2-service.widevine
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/bin/hw/android.hardware.nfc@1.2-service.sec
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/bin/hw/android.hardware.secure_element@1.1-service-uicc
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/bin/hw/vendor.dolby.hardware.dms@1.0-service
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/bin/hw/vendor.samsung.hardware.gnss@1.0-service
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/hw/vendor.egistec.hardware.fingerprint@4.0-impl.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/libGrallocWrapper.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/libsitril-client.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/libwvhidl.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/nfc_nci_sec.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/vendor.dolby.hardware.dms@1.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/vendor.egistec.hardware.fingerprint@4.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/vendor.samsung_slsi.telephony.hardware.oemservice@1.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/vendor.samsung_slsi.telephony.hardware.radio@1.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/vendor.samsung_slsi.telephony.hardware.radio@1.1.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib/vendor.samsung_slsi.telephony.hardware.radioExternal@1.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/hw/android.hardware.gnss@1.0-impl.samsung.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/hw/android.hardware.gnss@1.1-impl.samsung.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/hw/android.hardware.gnss@2.0-impl.samsung.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/hw/vendor.egistec.hardware.fingerprint@4.0-impl.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/hw/vendor.samsung.hardware.gnss@1.0-impl.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/libGrallocWrapper.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/libsitril-client.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/nfc_nci_sec.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/vendor.dolby.hardware.dms@1.0-impl.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/vendor.dolby.hardware.dms@1.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/vendor.egistec.hardware.fingerprint@4.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/vendor.samsung.hardware.gnss@1.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/vendor.samsung_slsi.telephony.hardware.oemservice@1.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/vendor.samsung_slsi.telephony.hardware.radio@1.0.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/vendor.samsung_slsi.telephony.hardware.radio@1.1.so
patchelf --remove-needed libhidltransport.so $BLOB_ROOT/vendor/lib64/vendor.samsung_slsi.telephony.hardware.radioExternal@1.0.so

# Remove libhwbinder dependency
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/bin/hw/android.hardware.drm@1.2-service.widevine
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/bin/hw/android.hardware.nfc@1.2-service.sec
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/bin/hw/android.hardware.secure_element@1.1-service-uicc
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib/libsitril-client.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib/libwvhidl.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib/vendor.dolby.hardware.dms@1.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib/vendor.egistec.hardware.fingerprint@4.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib/vendor.samsung_slsi.telephony.hardware.oemservice@1.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib/vendor.samsung_slsi.telephony.hardware.radio@1.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib/vendor.samsung_slsi.telephony.hardware.radio@1.1.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib/vendor.samsung_slsi.telephony.hardware.radioExternal@1.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/hw/android.hardware.gnss@1.0-impl.samsung.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/hw/android.hardware.gnss@1.1-impl.samsung.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/hw/android.hardware.gnss@2.0-impl.samsung.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/hw/vendor.samsung.hardware.gnss@1.0-impl.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/libsitril-client.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/vendor.dolby.hardware.dms@1.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/vendor.egistec.hardware.fingerprint@4.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/vendor.samsung.hardware.gnss@1.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/vendor.samsung_slsi.telephony.hardware.oemservice@1.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/vendor.samsung_slsi.telephony.hardware.radio@1.0.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/vendor.samsung_slsi.telephony.hardware.radio@1.1.so
patchelf --remove-needed libhwbinder.so $BLOB_ROOT/vendor/lib64/vendor.samsung_slsi.telephony.hardware.radioExternal@1.0.so

patchelf --replace-needed libprotobuf-cpp-lite.so libprotobuf-cpp-lite-v29.so $BLOB_ROOT/vendor/lib/libwvhidl.so

"${MY_DIR}/setup-makefiles.sh"
