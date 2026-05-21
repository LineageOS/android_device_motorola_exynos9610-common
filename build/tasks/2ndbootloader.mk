#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

ifneq ($(filter kane troika, $(TARGET_DEVICE)),)
$(INSTALLED_2NDBOOTLOADER_TARGET): $(INSTALLED_KERNEL_TARGET) | $(ACP)
	$(hide) $(ACP) $(PRODUCT_OUT)/obj/KERNEL_OBJ/arch/arm64/boot/dts/exynos/exynos9610.dtb $@
endif
