#ifndef _DS_AUDIO_CAPTURE_DEVICE_H_
#define _DS_AUDIO_CAPTURE_DEVICE_H_

#include "DSCaptureDevice.h"

class DSAudioCaptureDevice : public DSCaptureDevice
{
public:
    DSAudioCaptureDevice();

    ~DSAudioCaptureDevice();

    // -----------------------------------------------------------------------
    // set和get数据成员

    void SetSamplesPerSec(DWORD samsPerSec) { sams_per_sec_ = samsPerSec; }
    DWORD GetSamplesPerSec() { return sams_per_sec_; }

    void SetBitsPerSample(WORD bitsPerSample) { bits_per_sam_ = bitsPerSample; }
    WORD GetBitsPerSample() { return bits_per_sam_; }

    void SetChannels(WORD channels) { channels_ = channels; }
    WORD GetChannels() { return channels_; }

    void SetPreferredSamplesPerSec(DWORD samsPerSec) { preferred_sams_per_sec_ = samsPerSec; }
    DWORD GetPreferredSamplesPerSec() { return preferred_sams_per_sec_; }

    void SetPreferredBitsPerSample(WORD bitsPerSample) { preferred_bits_per_sam_ = bitsPerSample; }
    WORD GetPreferredBitsPerSample() { return preferred_bits_per_sam_; }

    void SetPreferredChannels(WORD channels) { preferred_channels_ = channels; }
    WORD GetPreferredChannels() { return preferred_channels_; }

private:
    DWORD sams_per_sec_;
    WORD bits_per_sam_;
    WORD channels_;

    DWORD preferred_sams_per_sec_;
    WORD preferred_bits_per_sam_;
    WORD preferred_channels_;
};

#endif // _DS_AUDIO_CAPTURE_DEVICE_H_
