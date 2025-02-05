/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef __EXYNOS_AUDIOPROXY_ABOX_H__
#define __EXYNOS_AUDIOPROXY_ABOX_H__


/* A-Box HW limitations */

// Supported Sampling Rate
#define MAX_NUM_PLAYBACK_SR     7
unsigned int supported_playback_samplingrate[MAX_NUM_PLAYBACK_SR] = {8000, 16000, 32000, 44100, 48000, 96000, 192000};

/* In spite of A-Box spec, we need to fix 48KHz recording only to support some solution limitation */
#define MAX_NUM_CAPTURE_SR      1
unsigned int supported_capture_samplingrate[MAX_NUM_CAPTURE_SR] = {48000};

// Supported Channel Mask
#define MAX_NUM_PLAYBACK_CM     2
audio_channel_mask_t supported_playback_channelmask[MAX_NUM_PLAYBACK_CM] = {AUDIO_CHANNEL_OUT_MONO, AUDIO_CHANNEL_OUT_STEREO};

#define MAX_NUM_CAPTURE_CM      2
audio_channel_mask_t supported_capture_channelmask[MAX_NUM_CAPTURE_CM] = {AUDIO_CHANNEL_IN_STEREO, AUDIO_CHANNEL_IN_FRONT_BACK};

// Supported PCM Format
#define MAX_NUM_PLAYBACK_PF     1
audio_format_t supported_playback_pcmformat[MAX_NUM_PLAYBACK_PF] = {AUDIO_FORMAT_PCM_16_BIT};

#define MAX_NUM_CAPTURE_PF      1
audio_format_t supported_capture_pcmformat[MAX_NUM_CAPTURE_PF] = {AUDIO_FORMAT_PCM_16_BIT};

// Supported Audio Format
#define MAX_NUM_PLAYBACK_AF     1
audio_format_t supported_playback_audioformat[MAX_NUM_PLAYBACK_AF] = {AUDIO_FORMAT_MP3};

/* Calliope Firmware Dump */
#define CALLIOPE_LOG_BUFFERSIZE     (4 * 1024)

#define CALLIOPE_DBG_PATH  "/sys/kernel/debug/abox/"
#define CALLIOPE_LOG       "log-00"
#define SYSFS_PREFIX       "/sys"
#define ABOX_DEV           "/devices/platform/14a50000.abox/"
#define ABOX_DEBUG         "0.abox-debug/"
#define ABOX_SRAM          "calliope_sram"
#define ABOX_DRAM          "calliope_dram"
#define ABOX_DUMP          "/data/vendor/log/abox/"
#define ABOX_DUMP_LIMIT (10)
#define ABOX_GPR           "gpr"

/*
 * Built-In Microphones Characteristics
 * It should be modified based on real target board's buili-in microphones configurations.
 */
// Main MIC(1st MIC)
struct audio_microphone_characteristic_t built_in_mic1 = {
    .device_id                  = "builtin_mic1",
    .id                         = 0,
    .device                     = AUDIO_DEVICE_IN_BUILTIN_MIC,
    .address                    = "bottom",
    .channel_mapping            = {AUDIO_MICROPHONE_CHANNEL_MAPPING_UNUSED},
    .location                   = AUDIO_MICROPHONE_LOCATION_MAINBODY,
    .group                      = 0,
    .index_in_the_group         = 0,
    .sensitivity                = -37.0,
    .max_spl                    = 132.5,
    .min_spl                    = 28.5,
    .directionality             = AUDIO_MICROPHONE_DIRECTIONALITY_OMNI,
    .num_frequency_responses    = 93,
    .frequency_responses[0]     = {97.16,102.92,109.02,115.48,122.32,129.57,137.25,145.38,153.99,163.12,172.78,183.02,193.87,205.35,217.52,230.41,244.06,258.52,273.84,290.07,307.26,325.46,344.75,365.17,386.81,409.73,434.01,459.73,486.97,515.82,546.39,578.76,613.06,649.38,687.86,728.62,771.79,817.52,865.96,917.28,971.63,1029.20,1090.18,1154.78,1223.21,1295.69,1372.46,1453.78,1539.93,1631.17,1727.83,1830.21,1938.65,2053.53,2175.20,2304.09,2440.62,2585.23,2738.42,2900.68,3072.56,3254.62,3447.47,3651.74,3868.12,4097.32,4340.10,4597.27,4869.68,5158.22,5463.87,5787.62,6130.56,6493.82,6878.60,7286.18,7717.92,8175.23,8659.64,9172.76,9716.28,10292.01,10901.84,11547.82,12232.07,12956.87,13724.61,14537.84,15399.27,16311.73,17278.26,18302.06,19386.53},
    .frequency_responses[1]     = {-0.40,-0.40,-0.60,-0.70,-0.40,-0.40,-0.30,-0.30,-0.30,-0.30,-0.20,-0.60,-0.90,-0.90,-1.00,-0.70,-0.80,-0.70,-0.70,-0.90,-0.70,-0.20,0.60,1.40,1.70,0.80,-0.80,-2.10,-2.30,-1.70,-0.90,0.50,1.30,1.20,0.80,0.10,0.20,0.40,2.30,2.40,0.00,-0.40,-0.10,0.70,-0.40,1.00,0.50,1.40,2.40,2.00,2.50,2.70,1.70,1.40,1.70,-1.90,-3.60,1.70,2.30,0.00,0.80,-0.30,0.60,1.90,1.40,-1.90,0.30,1.70,-0.60,0.40,2.20,3.60,-4.20,2.50,3.60,8.10,-4.30,5.70,7.30,9.60,7.80,10.20,16.40,18.60,20.10,22.50,23.50,17.60,17.90,18.80,17.70,15.10,14.70},
    .geometric_location.x       = 0.0513,
    .geometric_location.y       = 0.0,
    .geometric_location.z       = 0.0038,
    .orientation.x              = 0.0,
    .orientation.y              = -1.0,
    .orientation.z              = 0.0,
};

// Back MIC(2nd MIC)
struct audio_microphone_characteristic_t built_in_mic2 = {
    .device_id                  = "builtin_mic2",
    .id                         = 0,
    .device                     = AUDIO_DEVICE_IN_BACK_MIC,
    .address                    = "back",
    .channel_mapping            = {AUDIO_MICROPHONE_CHANNEL_MAPPING_UNUSED},
    .location                   = AUDIO_MICROPHONE_LOCATION_MAINBODY,
    .group                      = 0,
    .index_in_the_group         = 0,
    .sensitivity                = -37.0,
    .max_spl                    = 132.5,
    .min_spl                    = 28.5,
    .directionality             = AUDIO_MICROPHONE_DIRECTIONALITY_OMNI,
    .num_frequency_responses    = 93,
    .frequency_responses[0]     = {97.16,102.92,109.02,115.48,122.32,129.57,137.25,145.38,153.99,163.12,172.78,183.02,193.87,205.35,217.52,230.41,244.06,258.52,273.84,290.07,307.26,325.46,344.75,365.17,386.81,409.73,434.01,459.73,486.97,515.82,546.39,578.76,613.06,649.38,687.86,728.62,771.79,817.52,865.96,917.28,971.63,1029.20,1090.18,1154.78,1223.21,1295.69,1372.46,1453.78,1539.93,1631.17,1727.83,1830.21,1938.65,2053.53,2175.20,2304.09,2440.62,2585.23,2738.42,2900.68,3072.56,3254.62,3447.47,3651.74,3868.12,4097.32,4340.10,4597.27,4869.68,5158.22,5463.87,5787.62,6130.56,6493.82,6878.60,7286.18,7717.92,8175.23,8659.64,9172.76,9716.28,10292.01,10901.84,11547.82,12232.07,12956.87,13724.61,14537.84,15399.27,16311.73,17278.26,18302.06,19386.53},
    .frequency_responses[1]     = {-0.40,-0.40,-0.60,-0.70,-0.40,-0.40,-0.30,-0.30,-0.30,-0.30,-0.20,-0.60,-0.90,-0.90,-1.00,-0.70,-0.80,-0.70,-0.70,-0.90,-0.70,-0.20,0.60,1.40,1.70,0.80,-0.80,-2.10,-2.30,-1.70,-0.90,0.50,1.30,1.20,0.80,0.10,0.20,0.40,2.30,2.40,0.00,-0.40,-0.10,0.70,-0.40,1.00,0.50,1.40,2.40,2.00,2.50,2.70,1.70,1.40,1.70,-1.90,-3.60,1.70,2.30,0.00,0.80,-0.30,0.60,1.90,1.40,-1.90,0.30,1.70,-0.60,0.40,2.20,3.60,-4.20,2.50,3.60,8.10,-4.30,5.70,7.30,9.60,7.80,10.20,16.40,18.60,20.10,22.50,23.50,17.60,17.90,18.80,17.70,15.10,14.70},
    .geometric_location.x       = 0.0513,
    .geometric_location.y       = 0.0,
    .geometric_location.z       = 0.0038,
    .orientation.x              = 0.0,
    .orientation.y              = -1.0,
    .orientation.z              = 0.0,
};

// Third MIC(3rd MIC)


#define SOUND_TRIGGER_HAL_LIBRARY_PATH "/vendor/lib/hw/sound_trigger.primary.erd9610.so"

#endif  // __EXYNOS_AUDIOPROXY_ABOX_H__
