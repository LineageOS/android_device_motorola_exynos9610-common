#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixup_remove_arch_suffix,
    lib_fixups_user_type,
    lib_fixup_vendorcompat,
    libs_clang_rt_ubsan,
    libs_proto_3_9_1,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/motorola/exynos9610-common',
    'hardware/samsung_slsi-linaro/graphics',
    'hardware/samsung_slsi-linaro/interfaces',
    'hardware/samsung_slsi-linaro/exynos',
    'hardware/samsung_slsi-linaro/exynos/tee/kinibi410',
]

lib_fixups: lib_fixups_user_type = {
    libs_clang_rt_ubsan: lib_fixup_remove_arch_suffix,
    libs_proto_3_9_1: lib_fixup_vendorcompat,
}

blob_fixups: blob_fixups_user_type = {
    'system/app/ShannonIms/ShannonIms.apk': blob_fixup()
        .apktool_patch('blob-patches/ShannonIms', '-r'),
    'system/lib64/libmediaadaptor.so': blob_fixup()
        .add_needed('libmediaadaptor_shim.so'),
    'system_ext/app/ShannonDataService/ShannonDataService.apk': blob_fixup()
        .apktool_patch('blob-patches/ShannonDataService', '-r'),
    'system_ext/app/ShannonNetworkService/ShannonNetworkService.apk': blob_fixup()
        .apktool_patch('blob-patches/ShannonNetworkService', '-r'),
    'system_ext/app/ShannonQualifiedNetworksService/ShannonQualifiedNetworksService.apk': blob_fixup()
        .apktool_patch('blob-patches/ShannonQualifiedNetworksService', '-r'),
    ('vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service-rbs', 'vendor/lib64/hw/android.hardware.gnss@1.0-impl.samsung.so', 'vendor/lib64/hw/android.hardware.gnss@1.1-impl.samsung.so', 'vendor/lib64/hw/android.hardware.gnss@2.0-impl.samsung.so', 'vendor/lib64/hw/vendor.samsung.hardware.gnss@1.0-impl.so'): blob_fixup()
        .remove_needed('libhidltransport.so')
        .remove_needed('libhwbinder.so'),
    'vendor/bin/wfc-pkt-router': blob_fixup()
        .replace_needed('libnetutils.so', 'libip_checksum_shim.so'),
    'vendor/etc/permissions/com.motorola.motosignature.xml': blob_fixup()
        .regex_replace('system', 'vendor'),
    'vendor/lib/libaudioproxy.so': blob_fixup()
        .add_needed('libaudioproxy_shim.so'),
    ('vendor/lib/libhifills.so', 'vendor/lib64/libhifills.so'): blob_fixup()
        .add_needed('libdemangle.so')
        .add_needed('libprocessgroup.so'),
    ('vendor/lib/libwvhidl.so', 'vendor/lib64/libstrongswan.so'): blob_fixup()
        .add_needed('libcrypto_shim.so'),
    ('vendor/lib/sensors.rp.so', 'vendor/lib64/sensors.rp.so'): blob_fixup()
        .replace_needed('libutils.so', 'libutils-v32.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'exynos9610-common',
    'motorola',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
