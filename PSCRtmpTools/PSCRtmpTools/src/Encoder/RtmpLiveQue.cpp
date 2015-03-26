#include "stdafx.h"
#include "RtmpLiveQue.h"
#include "..\librtmp\AmfByteStream.h"

DECLARE_SINGLETON_MEMBER(CRtmpLiveQue);

CRtmpLiveQue::CRtmpLiveQue() :
	time_begin_(0)
	, last_timestamp_(0)
{

}

CRtmpLiveQue::~CRtmpLiveQue()
{

}

void CRtmpLiveQue::OnCaptureAudioBuffer(base::DataBuffer* dataBuf, unsigned int timestamp)
{
	//if (is_stop_ || false == is_enable_audio_) return;
	
	m_que.push( RtmpDataBuffer(FLV_TAG_TYPE_AUDIO, dataBuf->Clone(), GetTimestamp(), false));
	//m_que.push_back(RtmpDataBuffer(FLV_TAG_TYPE_AUDIO,dataBuf->Clone(), GetTimestamp(), false));

	delete dataBuf;
}

void CRtmpLiveQue::OnCaptureVideoBuffer(base::DataBuffer* dataBuf/*, unsigned int timestamp*/, bool isKeyframe)
{
	//if (is_stop_ || false == is_enable_video_) return;

	m_que.push(RtmpDataBuffer(FLV_TAG_TYPE_VIDEO, dataBuf->Clone(), GetTimestamp(), isKeyframe));
	//m_que.push_back(RtmpDataBuffer(FLV_TAG_TYPE_VIDEO, dataBuf->Clone(), GetTimestamp(), isKeyframe));


	delete dataBuf;
}

unsigned int CRtmpLiveQue::GetTimestamp()
{
	unsigned int timestamp;
	__int64 now = ::GetTickCount();

	//if (now < last_timestamp_)
	if (now < time_begin_)
	{
		timestamp = 0;
		last_timestamp_ = now;
		time_begin_ = now;
	}
	else
	{
		timestamp = now - time_begin_;
		//timestamp = now - last_timestamp_;
	}
	return timestamp;
}


