#ifndef _DS_CAPTURE_H_
#define _DS_CAPTURE_H_

#include "base/DataBuffer.h"
#include <vector>

#include "DShow/DSGraph.h"

class DSAudioGraph;
class DSVideoGraph;
class DSAudioCaptureDevice;
class DSVideoCaptureDevice;
class AudioEncoderThread;
class VideoEncoderThread;
//class LMRTPToDarwinThread;

struct DSAudioFormat
{
    int samples_per_sec;
    int bits_per_sample;
    int channels;
};

struct DSVideoFormat
{
    int width;
    int height;
    int fps;
    int bitrate;
};

class DSCaptureListener
{
public:
    virtual ~DSCaptureListener() {}

    virtual void OnCaptureAudioBuffer(base::DataBuffer* dataBuf, unsigned int timestamp) = 0;

    virtual void OnCaptureVideoBuffer(base::DataBuffer* dataBuf, unsigned int timestamp, bool isKeyframe) = 0;

    virtual void OnSPSAndPPS(char* spsBuf, int spsSize, char* ppsBuf, int ppsSize) = 0;
};

class DSCapture
{
public:
    static void ListAudioCapDevices(std::map<CString, CString>& deviceList);

    static void ListVideoCapDevices(std::map<CString, CString>& deviceList);

    static void ListVideoCapDeviceWH(const CString& vDeviceID, 
        std::vector<int>& widthList, std::vector<int>& heightList);

public:
    DSCapture();

    ~DSCapture();

    void Create(const CString& audioDeviceID, const CString& videoDeviceID,
        const DSAudioFormat& audioFormat, const DSVideoFormat& videoFormat,
        const CString& audioOutname, const CString& videoOutname);

    void SetListener(DSCaptureListener* listener);

    void Destroy();

    HRESULT	SetVideoFormat(UINT nPreferredVideoWidth, 
        UINT nPreferredVideoHeight, REFERENCE_TIME rtPreferredVideoFPS);

    void StartVideo();

    void StopVideo();

    HRESULT	SetAudioFormat(DWORD dwPreferredSamplesPerSec,
        WORD wPreferredBitsPerSample, WORD nPreferredChannels);

    void StartAudio();

    void StopAudio();

    void AdjustVideoWindow(OAHWND owner, unsigned int width, unsigned int height);

    void SavePicture();

    DSVideoGraph* GetVideoGraph() { return ds_video_graph_; }

    DSAudioGraph* GetAudioGraph() { return ds_audio_graph_; }

    DSGraph * DShowGraph() { return ds_graph_; }

private:
    std::string CStringToString(const CString& mfcStr);

private:
    DSGraph*		ds_graph_;

    DSVideoCaptureDevice*	ds_video_cap_device_;
    DSAudioCaptureDevice*	ds_audio_cap_device_;

    DSVideoGraph*			ds_video_graph_;
    DSAudioGraph*			ds_audio_graph_;

    AudioEncoderThread* audio_encoder_thread_;
    VideoEncoderThread* video_encoder_thread_;
};

#endif // _DS_CAPTURE_H_
