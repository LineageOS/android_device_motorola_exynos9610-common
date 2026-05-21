/*
 * SPDX-FileCopyrightText: 2017 The Android Open Source Project
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef AUDIO_PROXY_H
#define AUDIO_PROXY_H

#include <system/audio.h>
#include <hardware/hardware.h>
#include <hardware/audio.h>
#include <hardware/audio_alsaops.h>

#include <audio_utils/resampler.h>

#include "alsa_device_profile.h"
#include "alsa_device_proxy.h"
#include "alsa_logging.h"

#include "audio_streams.h"
#include "audio_usages.h"
#include "audio_devices.h"
#include "audio_offload.h"

#include "audio_pcm.h"
#include "audio_mixer.h"
#include "audio_abox.h"
#include "audio_streamconfig.h"



/* Data Structure for Audio Proxy */
struct audio_proxy_stream
{
    audio_stream_type stream_type;
    audio_usage       stream_usage;

    // Real configuration for PCM/Compress Device
    int sound_card;
    int sound_device;

    struct pcm *pcm;
    struct pcm_config pcmconfig;

    // Offload Specific
    struct compress *compress;
    struct compr_config comprconfig;

    int nonblock_flag;
    int ready_new_metadata;
    struct compr_gapless_mdata offload_metadata;

    // USB Specific
    alsa_device_profile *usb_profile;
    alsa_device_proxy   *usb_proxy;

    // Common
    unsigned int            requested_sample_rate;
    audio_channel_mask_t    requested_channel_mask;
    audio_format_t          requested_format;

    float vol_left, vol_right;

    uint64_t frames; /* total frames written, not cleared when entering standby */


    // Channel Conversion & Resample for Recording
    bool   need_monoconversion;
    bool   need_resampling;

    int16_t* actual_read_buf;
    int      actual_read_status;
    size_t   actual_read_buf_size;
    size_t   read_buf_frames;

    void *   proc_buf_out;
    int      proc_buf_size;

    // Resampler
    struct resampler_itfe *             resampler;
    struct resampler_buffer_provider    buf_provider;

#ifdef SUPPORT_STHAL_INTERFACE
    int soundtrigger_handle;
#ifdef SEAMLESS_DUMP
    FILE *fp;
#endif
#endif
    int cpcall_rec_skipcnt;
    bool callrec_err_detect;

};

struct audio_proxy
{
    // Audio Path Routing
    struct mixer *mixer;
    struct audio_route *aroute;
    char *xml_path;

    // Mixer Update Thread
    pthread_rwlock_t mixer_update_lock;
    pthread_t        mixer_update_thread;

    audio_usage   active_playback_ausage;
    device_type   active_playback_device;
    modifier_type active_playback_modifier;

    audio_usage   active_capture_ausage;
    device_type   active_capture_device;
    modifier_type active_capture_modifier;

    // Primary Output Stream Proxy
    struct audio_proxy_stream *primary_out;

    /* Device Configuration */
    int num_earpiece;
    int num_speaker;
    int num_proximity;

    /* BuiltIn MIC Characteristics Map */
    int num_mic;
    struct audio_microphone_characteristic_t *mic_map;

    // PCM Devices for Audio Path(Loopback / ERAP)
    bool support_out_loopback;
    struct pcm *out_loopback;
    struct pcm *erap_in;

    /* Speaker AMP Configuration */
    bool support_spkamp;
    struct pcm *spkamp_reference;
    struct pcm *spkamp_playback;

    /*  Bluetooth Configuration */
    bool bt_internal;
    bool bt_external;

    bool support_btsco;
    struct pcm *btsco_playback;
    struct pcm *btsco_erap_pcminfo[BTSCO_MAX_ERAP_IDX];
    unsigned int btsco_erap_flag[BTSCO_MAX_ERAP_IDX];
    int btsco_samplerate;
    bool btsco_nrec;

    /* FM Radio Configuration */
    bool fm_internal;
    bool fm_external;

    // PCM Devices for FM Radio
    struct pcm *fm_playback;     // FM PCM Playback from A-Box
    struct pcm *fm_capture;      // FM PCM Capture to A-Box

    // ALSA Profiles for USB
    alsa_device_profile usb_out;
    alsa_device_profile usb_in;

    /* PCM Devices for Voice Call */
    struct pcm *call_rx;    // CP to Output Devices
    struct pcm *call_tx;    // Input Devices to CP

    // Call State
    bool call_state;

    // Audio Mode
    int audio_mode;

    /* Call Screen Loopback Configuration */
    struct pcm *callscreen_loopback;

    // Voice WakeUp
#ifdef SUPPORT_STHAL_INTERFACE
    /* SoundTrigger library interface */
    void *sound_trigger_lib;
    int (*sound_trigger_open_for_streaming)();
    size_t (*sound_trigger_read_samples)(int, void*, size_t);
    int (*sound_trigger_close_for_streaming)(int);
    int (*sound_trigger_open_recording)();
    size_t (*sound_trigger_read_recording_samples)(void*, size_t);
    int (*sound_trigger_close_recording)();

    int (*sound_trigger_headset_status)(int);
    int (*sound_trigger_voicecall_status)(int);

    int sthal_state;
#endif

    void *offload_effect_lib;
    void (*offload_effect_lib_update)(struct mixer *, int);
};

int proxy_get_mixer_value_int(void *proxy, const char *name);

#define AUDIO_PARAMETER_DEVICE_CARD   "card"
#define AUDIO_PARAMETER_DEVICE_DEVICE "device"

#define MIXER_UPDATE_TIMEOUT    5  // 5 seconds


// Definition for MMAP Stream
#define MMAP_PERIOD_SIZE (DEFAULT_MEDIA_SAMPLING_RATE/1000)
#define MMAP_PERIOD_COUNT_MIN 32
#define MMAP_PERIOD_COUNT_MAX 512
#define MMAP_PERIOD_COUNT_DEFAULT (MMAP_PERIOD_COUNT_MAX)

#endif /* AUDIO_PROXY_H */
