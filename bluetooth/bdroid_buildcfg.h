/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#define BTM_WBS_INCLUDED TRUE       /* Enable WBS */
#define BTIF_HF_WBS_PREFERRED FALSE /* Don't prefer WBS    */

/*
 * Toggles support for vendor specific extensions such as RPA offloading,
 * feature discovery, multi-adv etc.
 */
#define BLE_VND_INCLUDED TRUE

#define BTM_SCO_ENHANCED_SYNC_ENABLED FALSE

#define BTM_ESCO_TRANSPORT_UNIT_SIZE_PCM16

#endif
