#ifndef _AUDIO_ENCODER_THREAD_H_
#define _AUDIO_ENCODER_THREAD_H_
#include <string>
#include "FAACEncoder.h"
#include "..\dshow\DSAudioCaptureDevice.h"
#include "RtmpLiveQue.h"

//class DSAudioCaptureDevice;

class AudioEncoderThread: public EcoderCtr
{
public:
	AudioEncoderThread(){};
	~AudioEncoderThread(){};

	void Init();
	void Run(){};//¿ªÊ¼

	int Create(DSAudioCaptureDevice* audioCapDevice){};

    //void SetListener(DSCaptureListener* listener) { listener_ = listener; }

	void SetOutputFilename(const std::string& filename){};

private:
    FAACEncoder* faac_encoder_;
    std::string filename_aac_;
	std::shared_ptr<CRtmpLiveQue> m_pRtmpLiveQue;
};


#endif // _AUDIO_ENCODER_THREAD_H_
