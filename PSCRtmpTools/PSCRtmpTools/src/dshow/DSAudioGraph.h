#ifndef _DS_AUDIO_GRAPH_H_
#define _DS_AUDIO_GRAPH_H_

#include "DSGraph.h"
#include "DSAudioCaptureDevice.h"

class DSAudioGraph : public DSGraph
{
public:
    DSAudioGraph();

    ~DSAudioGraph();

    int Create(DSAudioCaptureDevice* audioCapDevice);

    void Destroy();

    DWORD GetSamplesPerSec() { return audio_cap_device_->GetSamplesPerSec(); }

    WORD GetBitsPerSample() { return audio_cap_device_->GetBitsPerSample(); }

    WORD GetChannels() { return audio_cap_device_->GetChannels(); }

private:
    HRESULT SetAudioFormat(DWORD samsPerSec, WORD bitsPerSam, WORD channels);

private:
    DSAudioCaptureDevice* audio_cap_device_;
};

#endif // _DS_AUDIO_GRAPH_H_
