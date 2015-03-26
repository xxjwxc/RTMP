#include "stdafx.h"
#include "DSAudioGraph.h"
#include "baseclasses/mtype.h"

DSAudioGraph::DSAudioGraph()
    : audio_cap_device_(NULL)
{

}

DSAudioGraph::~DSAudioGraph()
{
    Destroy();
}

int DSAudioGraph::Create(DSAudioCaptureDevice* audioCapDevice)
{
    audio_cap_device_ = audioCapDevice;

    SetCapDevice(CLSID_AudioInputDeviceCategory, audio_cap_device_->GetID());

    SetAudioFormat(audio_cap_device_->GetPreferredSamplesPerSec(),
        audio_cap_device_->GetPreferredBitsPerSample(), 
        audio_cap_device_->GetPreferredChannels());

    if (FAILED(graph_->AddFilter(device_filter_, L"Source" )))
        return -1;

    if (FAILED(graph_->AddFilter(grabber_filter_, L"Grabber")))
        return -1;

    CMediaType GrabType;
    GrabType.SetType(&MEDIATYPE_Audio);
    GrabType.SetSubtype(&MEDIASUBTYPE_PCM);
    if (FAILED(sample_grabber_->SetMediaType( &GrabType)))
        return -1;

    if (FAILED(sample_grabber_->SetBufferSamples(TRUE)))
        return -1;
    sample_grabber_->SetOneShot(FALSE);

    if (FAILED(sample_grabber_->SetCallback((ISampleGrabberCB*)&grabber_callback_, 1)))
        return -1;

    if (FAILED(graph_->AddFilter(null_render_filter_, L"NullRenderer")))
        return -1;

    if ( FAILED(capture_builder_->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, 
        device_filter_, grabber_filter_, null_render_filter_)) )
    {
        return -1;
    }

    AM_MEDIA_TYPE mt;
    if (FAILED(sample_grabber_->GetConnectedMediaType(&mt)))
        return -1;

    WAVEFORMATEX *wf = (WAVEFORMATEX *)mt.pbFormat;
    audio_cap_device_->SetBitsPerSample(wf->wBitsPerSample);
    audio_cap_device_->SetSamplesPerSec(wf->nSamplesPerSec);
    audio_cap_device_->SetChannels(wf->nChannels);
    FreeMediaType(mt);

    if (FAILED(graph_->QueryInterface(IID_IMediaControl, (void **)&media_control_)))
        return 0;

    if (FAILED(graph_->QueryInterface(IID_IMediaEvent, (void **)&media_event_)))
        return 0;

    return 0;
}

void DSAudioGraph::Destroy()
{

}

HRESULT DSAudioGraph::SetAudioFormat(DWORD samsPerSec, WORD bitsPerSam, WORD channels)
{
    HRESULT hRet;
    WORD wBytesPerSample = bitsPerSam / 8;
    DWORD dwBytesPerSecond = wBytesPerSample * samsPerSec * channels;
    DWORD dwBufferSize = 1024 * channels * wBytesPerSample;

    IAMStreamConfig *pConfig = NULL;
    if (FAILED(hRet = capture_builder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, 
        device_filter_, IID_IAMStreamConfig, (void**)&pConfig)))
    {
        return -1;
    }

    IAMBufferNegotiation *pNeg;
    if (FAILED(hRet = capture_builder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, 
        device_filter_, IID_IAMBufferNegotiation, (void**)&pNeg)))
    {
        return -1;
    }

    int iCount = 0, iSize  = 0;
    if (FAILED(pConfig->GetNumberOfCapabilities(&iCount, &iSize)))
        return -1;

    if (iSize == sizeof(AUDIO_STREAM_CONFIG_CAPS))
    {
        for (int iFormat = 0; iFormat < iCount; iFormat++)
        {
            AUDIO_STREAM_CONFIG_CAPS scc;
            AM_MEDIA_TYPE *pmtConfig;
            HRESULT hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);

            if (SUCCEEDED(hr))
            {
                if ((pmtConfig->majortype == MEDIATYPE_Audio) &&
                    (pmtConfig->formattype == FORMAT_WaveFormatEx) &&
                    (pmtConfig->cbFormat >= sizeof (WAVEFORMATEX)) &&
                    (pmtConfig->pbFormat != NULL))
                {
                    WAVEFORMATEX *wf = (WAVEFORMATEX *)pmtConfig->pbFormat;
                    if( ( wf->nSamplesPerSec == samsPerSec ) &&
                        ( wf->wBitsPerSample == bitsPerSam ) &&
                        ( wf->nChannels == channels ) )
                    {
                        ALLOCATOR_PROPERTIES prop={0};
                        prop.cbBuffer = dwBufferSize;
                        prop.cBuffers = 6;
                        prop.cbAlign = wBytesPerSample * channels;
                        pNeg->SuggestAllocatorProperties(&prop);

                        WAVEFORMATEX *wf = (WAVEFORMATEX *)pmtConfig->pbFormat;
                        wf->nAvgBytesPerSec = dwBytesPerSecond;
                        wf->nBlockAlign = wBytesPerSample * channels;
                        wf->nChannels = channels;
                        wf->nSamplesPerSec = samsPerSec;
                        wf->wBitsPerSample = bitsPerSam;

                        pConfig->SetFormat(pmtConfig);
                    }
                }

                DeleteMediaType(pmtConfig);
            }
        }
    }

    SAFE_RELEASE(pConfig);
    SAFE_RELEASE(pNeg);

    return 0;
}
