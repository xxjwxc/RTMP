#include "stdafx.h"
#include "AudioEncoder.h"

void AudioEncoderThread::Init()
{
	m_pRtmpLiveQue = CRtmpLiveQue::instance();
}
