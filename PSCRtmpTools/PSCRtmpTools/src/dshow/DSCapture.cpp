#include "stdafx.h"
#include "DSCapture.h"

#include <map>

#include "DSAudioGraph.h"
#include "DSVideoGraph.h"
#include "DSAudioCaptureDevice.h"
#include "DSVideoCaptureDevice.h"
#include "encoder/AudioEncoderThread.h"
#include "encoder/VideoEncoderThread.h"

void DSCapture::ListAudioCapDevices(std::map<CString, CString>& deviceList)
{
    return DSCaptureDevice::ListCapDevices(CLSID_AudioInputDeviceCategory, deviceList);
}

void DSCapture::ListVideoCapDevices(std::map<CString, CString>& deviceList)
{
    return DSCaptureDevice::ListCapDevices(CLSID_VideoInputDeviceCategory, deviceList);
}

void DSCapture::ListVideoCapDeviceWH(const CString& vDeviceID, 
    std::vector<int>& widthList, std::vector<int>& heightList)
{
    DSCaptureDevice::ListVideoCapDeviceWH(vDeviceID, widthList, heightList);
}

DSCapture::DSCapture()
{
    //ds_graph_ = new DSGraph;

    ds_video_graph_ = NULL;
    ds_audio_graph_ = NULL;

    ds_video_cap_device_ = NULL;
    ds_audio_cap_device_ = NULL;

    audio_encoder_thread_ = NULL;
    video_encoder_thread_ = NULL;
}

DSCapture::~DSCapture()
{
    Destroy();
    //SAFE_DELETE( ds_graph_ );
}

void DSCapture::Destroy()
{
    SAFE_DELETE( ds_video_graph_ );
    SAFE_DELETE( ds_audio_graph_ );
    SAFE_DELETE( ds_video_cap_device_ );
    SAFE_DELETE( ds_audio_cap_device_ );

    SAFE_DELETE( audio_encoder_thread_ );
    SAFE_DELETE( video_encoder_thread_ );
}

void DSCapture::Create(const CString& audioDeviceID, const CString& videoDeviceID,
    const DSAudioFormat& audioFormat, const DSVideoFormat& videoFormat,
    const CString& audioOutname, const CString& videoOutname)
{
    HRESULT hr;

    // audio
    if (ds_audio_cap_device_)
        delete ds_audio_cap_device_;
    if (false == audioDeviceID.IsEmpty())
    {
        ds_audio_cap_device_ = new DSAudioCaptureDevice;
        ds_audio_cap_device_->SetDeviceID(audioDeviceID);
        ds_audio_cap_device_->SetPreferredSamplesPerSec(audioFormat.samples_per_sec);
        ds_audio_cap_device_->SetPreferredBitsPerSample(audioFormat.bits_per_sample);
        ds_audio_cap_device_->SetPreferredChannels(audioFormat.channels);

        if (ds_audio_graph_)
            delete ds_audio_graph_;
        ds_audio_graph_ = new DSAudioGraph;
        hr = ds_audio_graph_->Create( ds_audio_cap_device_ );
        if( FAILED( hr ) )
        {

        }
        else
        {
            delete audio_encoder_thread_;
            audio_encoder_thread_ = new AudioEncoderThread(ds_audio_graph_);
        }

        if (audio_encoder_thread_)
            audio_encoder_thread_->SetOutputFilename(CStringToString(audioOutname));
    }

    // video
    if (ds_video_cap_device_)
        delete ds_video_cap_device_;
    if (false == videoDeviceID.IsEmpty())
    {
        ds_video_cap_device_ = new DSVideoCaptureDevice;
        ds_video_cap_device_->SetDeviceID(videoDeviceID);
        ds_video_cap_device_->SetPreferredVideoWidth(videoFormat.width);
        ds_video_cap_device_->SetPreferredVideoHeight(videoFormat.height);
        ds_video_cap_device_->SetPreferredVideoFPS(videoFormat.fps);

        if (ds_video_graph_)
            delete ds_video_graph_;
        ds_video_graph_ = new DSVideoGraph;
        hr = ds_video_graph_->Create(ds_video_cap_device_);
        if(FAILED(hr))
        {
            return;
        }

        delete video_encoder_thread_;
        video_encoder_thread_ = new VideoEncoderThread(ds_video_graph_, videoFormat.bitrate);

        if (video_encoder_thread_)
            video_encoder_thread_->SetOutputFilename(CStringToString(videoOutname));
    }
}

void DSCapture::SetListener(DSCaptureListener* listener)
{
    if (audio_encoder_thread_)
    {
        audio_encoder_thread_->SetListener(listener);
    }

    if (video_encoder_thread_)
    {
        video_encoder_thread_->SetListener(listener);
    }
}

void DSCapture::StartVideo()
{
    if (video_encoder_thread_) video_encoder_thread_->Start();
    if (ds_video_graph_) ds_video_graph_->Start();
}

void DSCapture::StopVideo()
{
    if (ds_video_graph_) ds_video_graph_->Stop();
    if (video_encoder_thread_)
    {
        video_encoder_thread_->Stop();
        video_encoder_thread_->Join();
    }
}

HRESULT DSCapture::SetVideoFormat( UINT nPreferredVideoWidth, UINT nPreferredVideoHeight, REFERENCE_TIME rtPreferredVideoFPS )
{
    StopVideo();

    ds_video_graph_->Destroy();

    // setting preferred width, height, fps
    // the video capture device's supported video format may not match it, that's
    // why these functions contain "preferred" in their names
    //
    ds_video_cap_device_->SetPreferredVideoWidth( nPreferredVideoWidth );
    ds_video_cap_device_->SetPreferredVideoHeight( nPreferredVideoHeight );
    ds_video_cap_device_->SetPreferredVideoFPS( rtPreferredVideoFPS );

    HRESULT hr = ds_video_graph_->Create( ds_video_cap_device_ );
    if( FAILED( hr ) )
    {
        return hr;
    }

    StartVideo();

    return 0;
}

void DSCapture::StartAudio()
{
    if (audio_encoder_thread_) audio_encoder_thread_->Start();
    if (ds_audio_graph_) ds_audio_graph_->Start();
}

void DSCapture::StopAudio()
{
    if (ds_audio_graph_) ds_audio_graph_->Stop();
    if (audio_encoder_thread_)
    {
        audio_encoder_thread_->Stop();
        audio_encoder_thread_->Join();
    }
}

HRESULT DSCapture::SetAudioFormat( DWORD dwPreferredSamplesPerSec, WORD wPreferredBitsPerSample, WORD nPreferredChannels )
{
    StopAudio();

    ds_audio_graph_->Destroy();

    ds_audio_cap_device_->SetPreferredSamplesPerSec( dwPreferredSamplesPerSec );
    ds_audio_cap_device_->SetPreferredBitsPerSample( wPreferredBitsPerSample );
    ds_audio_cap_device_->SetPreferredChannels( nPreferredChannels );

    HRESULT hr = ds_audio_graph_->Create( ds_audio_cap_device_ );
    if( FAILED( hr ) )
    {
        return hr;
    }

    StartAudio();

    return 0;
}

void DSCapture::AdjustVideoWindow(OAHWND owner, unsigned int width, unsigned int height)
{
    if (ds_video_graph_)
        ds_video_graph_->AdjustVideoWindow(owner, width, height);
}

std::string DSCapture::CStringToString(const CString& mfcStr)
{
    CT2CA pszConvertedAnsiString(mfcStr);
    return (pszConvertedAnsiString);
}

void DSCapture::SavePicture()
{
    if (video_encoder_thread_)
    {
        video_encoder_thread_->SavePicture();
    }
}
