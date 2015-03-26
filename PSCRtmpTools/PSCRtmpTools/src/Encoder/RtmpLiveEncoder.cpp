#include "stdafx.h"
#include "RtmpLiveEncoder.h"
#include "..\librtmp\AmfByteStream.h"
#include <winbase.h>
#include "..\librtmp\BitWritter.h"
#include "..\tools\MyIni.h"

CMyIni g_myIni;

bool RtmpLiveEncoder::s_VideoBegin = false;

RtmpLiveEncoder::RtmpLiveEncoder() :m_libRtmp(new LibRtmp(g_myIni.GetIntConfig("isLog"), g_myIni.GetIntConfig("isRecord"))),
	audio_mem_buf_size_(0),
	audio_mem_buf_(nullptr)
{
	source_samrate_ = 44100;
	source_channel_ = 1;

	m_width = 320;
	m_height = 240;//default data

	sps_ = NULL;
	sps_size_ = 0;
	pps_ = NULL;
	pps_size_ = 0;

	s_VideoBegin = false;

	m_pRtmpLiveQue = CRtmpLiveQue::instance();
}

RtmpLiveEncoder::~RtmpLiveEncoder()
{
	m_libRtmp.reset();

	if (sps_)
		free(sps_);
	if (pps_)
		free(pps_);

	if (audio_mem_buf_)
		delete[] audio_mem_buf_;
	if (video_mem_buf_)
		delete[] video_mem_buf_;
}

void RtmpLiveEncoder::Run()
{
	// 发送metadata包
	//SendMetadataPacket();

	// 开始捕获音视频
// 	if (is_enable_audio_) ds_capture_->StartAudio();
// 	if (is_enable_video_) ds_capture_->StartVideo();
	is_enable_video_ = true;

	if (false == is_enable_video_)
	{
		SendAACSequenceHeaderPacket();
		s_VideoBegin = true;

		m_pRtmpLiveQue->time_begin_ = ::GetTickCount();
		m_pRtmpLiveQue->last_timestamp_ = m_pRtmpLiveQue->time_begin_;
	}

	// 从队列中取出音频或视频数据
	RtmpDataBuffer rtmp_data;

	while (!m_is_stop)
	{
		if (m_is_pause)
		{
			Sleep(1000);
			continue;
		}

// 		if (m_pRtmpLiveQue->m_que.size() > 5)
// 		{
// 			//ASSERT(false);
// 			TRACE("no\n");
// 		}
		//if (!m_pRtmpLiveQue->m_que.empty())
		{
			rtmp_data = std::move(m_pRtmpLiveQue->m_que.pop());//弹出一个队列
			if (rtmp_data.data != NULL)
			{
				if (rtmp_data.type == FLV_TAG_TYPE_AUDIO)
					SendAudioDataPacket(rtmp_data.data, rtmp_data.timestamp);
				else
					SendVideoDataPacket(rtmp_data.data, rtmp_data.timestamp, rtmp_data.is_keyframe);
			}
		}

		if (m_pRtmpLiveQue->m_que.size() == 0)
			Sleep(10);//切换时间片
	}

// 	if (is_enable_video_) ds_capture_->StopVideo();
// 	if (is_enable_audio_) ds_capture_->StopAudio();

	m_libRtmp->Close();
}


void RtmpLiveEncoder::SendVideoDataPacket(base::DataBuffer* dataBuf, unsigned int timestamp, bool isKeyframe)
{
	int need_buf_size = dataBuf->BufLen() + 5;
	if (need_buf_size > video_mem_buf_size_)
	{
		if (video_mem_buf_)
			delete[] video_mem_buf_;
		video_mem_buf_ = new char[need_buf_size];
		video_mem_buf_size_ = need_buf_size;
	}

	char* buf = video_mem_buf_;
	char* pbuf = buf;

	unsigned char flag = 0;
	if (isKeyframe)
		flag = 0x17;
	else
		flag = 0x27;

	pbuf = UI08ToBytes(pbuf, flag);
	pbuf = UI08ToBytes(pbuf, 1);    // avc packet type (0, nalu)
	pbuf = UI24ToBytes(pbuf, 0);    // composition time

	memcpy(pbuf, dataBuf->Buf(), dataBuf->BufLen());
	pbuf += dataBuf->BufLen();

	m_libRtmp->Send(buf, (int)(pbuf - buf), FLV_TAG_TYPE_VIDEO, timestamp);

	delete dataBuf;
}

void RtmpLiveEncoder::SendAudioDataPacket(base::DataBuffer* dataBuf, unsigned int timestamp)
{
	int need_buf_size = dataBuf->BufLen() + 5;
	if (need_buf_size > audio_mem_buf_size_)
	{
		if (audio_mem_buf_)
			delete[] audio_mem_buf_;
		audio_mem_buf_ = new char[need_buf_size];
		audio_mem_buf_size_ = need_buf_size;
	}

	char* buf = audio_mem_buf_;
	char* pbuf = buf;

	unsigned char flag = 0;
	flag = (10 << 4) |  // soundformat "10 == AAC"
		(3 << 2) |      // soundrate   "3  == 44-kHZ"
		(1 << 1) |      // soundsize   "1  == 16bit"
		1;              // soundtype   "1  == Stereo"

	pbuf = UI08ToBytes(pbuf, flag);

	pbuf = UI08ToBytes(pbuf, 1);    // aac packet type (1, raw)

	memcpy(pbuf, dataBuf->Buf(), dataBuf->BufLen());
	pbuf += dataBuf->BufLen();

	m_libRtmp->Send(buf, (int)(pbuf - buf), FLV_TAG_TYPE_AUDIO, timestamp);

	delete dataBuf;
}

void RtmpLiveEncoder::Init(int width, int height)
{
	m_width = width;
	m_height = height;
	video_mem_buf_size_ = width*height * 3;
	video_mem_buf_ = new char[video_mem_buf_size_];
}

void RtmpLiveEncoder::SendMetadataPacket()
{
	char metadata_buf[4096];
	char* pbuf = WriteMetadata(metadata_buf);

	m_libRtmp->Send(metadata_buf, (int)(pbuf - metadata_buf), FLV_TAG_TYPE_META, 0);
}

void RtmpLiveEncoder::SendAVCSequenceHeaderPacket()
{
	char avc_seq_buf[4096];
	char* pbuf = WriteAVCSequenceHeader(avc_seq_buf);

	m_libRtmp->Send(avc_seq_buf, (int)(pbuf - avc_seq_buf), FLV_TAG_TYPE_VIDEO, 0);
}

void RtmpLiveEncoder::SendAACSequenceHeaderPacket()
{
	char aac_seq_buf[4096];
	char* pbuf = WriteAACSequenceHeader(aac_seq_buf);

	m_libRtmp->Send(aac_seq_buf, (int)(pbuf - aac_seq_buf), FLV_TAG_TYPE_AUDIO, 0);
}

char* RtmpLiveEncoder::WriteMetadata(char* buf)
{
	char* pbuf = buf;

	pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_STRING);
	pbuf = AmfStringToBytes(pbuf, "@setDataFrame");

	pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_STRING);
	pbuf = AmfStringToBytes(pbuf, "onMetaData");

	//     pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_MIXEDARRAY);
	//     pbuf = UI32ToBytes(pbuf, 2);
	pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_OBJECT);

	pbuf = AmfStringToBytes(pbuf, "width");
	pbuf = AmfDoubleToBytes(pbuf, m_width);

	pbuf = AmfStringToBytes(pbuf, "height");
	pbuf = AmfDoubleToBytes(pbuf, m_height);

	pbuf = AmfStringToBytes(pbuf, "framerate");
	pbuf = AmfDoubleToBytes(pbuf, 10);

	pbuf = AmfStringToBytes(pbuf, "videocodecid");
	pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_STRING);
	pbuf = AmfStringToBytes(pbuf, "avc1");

	// 0x00 0x00 0x09
	pbuf = AmfStringToBytes(pbuf, "");
	pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_OBJECT_END);

	return pbuf;
}

char* RtmpLiveEncoder::WriteAVCSequenceHeader(char* buf)
{
	char* pbuf = buf;

	unsigned char flag = 0;
	flag = (1 << 4) |   // frametype "1  == keyframe"
		7;              // codecid   "7  == AVC"

	pbuf = UI08ToBytes(pbuf, flag);

	pbuf = UI08ToBytes(pbuf, 0);    // avc packet type (0, header)
	pbuf = UI24ToBytes(pbuf, 0);    // composition time

	// AVCDecoderConfigurationRecord

	pbuf = UI08ToBytes(pbuf, 1);            // configurationVersion
	pbuf = UI08ToBytes(pbuf, sps_[1]);      // AVCProfileIndication
	pbuf = UI08ToBytes(pbuf, sps_[2]);      // profile_compatibility
	pbuf = UI08ToBytes(pbuf, sps_[3]);      // AVCLevelIndication
	pbuf = UI08ToBytes(pbuf, 0xff);         // 6 bits reserved (111111) + 2 bits nal size length - 1
	pbuf = UI08ToBytes(pbuf, 0xe1);         // 3 bits reserved (111) + 5 bits number of sps (00001)
	pbuf = UI16ToBytes(pbuf, sps_size_);    // sps
	memcpy(pbuf, sps_, sps_size_);
	pbuf += sps_size_;
	pbuf = UI08ToBytes(pbuf, 1);            // number of pps
	pbuf = UI16ToBytes(pbuf, pps_size_);    // pps
	memcpy(pbuf, pps_, pps_size_);
	pbuf += pps_size_;

	return pbuf;
}

char* RtmpLiveEncoder::WriteAACSequenceHeader(char* buf)
{
	char* pbuf = buf;

	unsigned char flag = 0;
	flag = (10 << 4) |  // soundformat "10 == AAC"
		(3 << 2) |      // soundrate   "3  == 44-kHZ"
		(1 << 1) |      // soundsize   "1  == 16bit"
		1;              // soundtype   "1  == Stereo"

	pbuf = UI08ToBytes(pbuf, flag);

	pbuf = UI08ToBytes(pbuf, 0);    // aac packet type (0, header)

	// AudioSpecificConfig

	int sample_rate_index;
	if (source_samrate_ == 48000)
		sample_rate_index = 0x03;
	else if (source_samrate_ == 44100)
		sample_rate_index = 0x04;
	else if (source_samrate_ == 32000)
		sample_rate_index = 0x05;
	else if (source_samrate_ == 24000)
		sample_rate_index = 0x06;
	else if (source_samrate_ == 22050)
		sample_rate_index = 0x07;
	else if (source_samrate_ == 16000)
		sample_rate_index = 0x08;
	else if (source_samrate_ == 12000)
		sample_rate_index = 0x09;
	else if (source_samrate_ == 11025)
		sample_rate_index = 0x0a;
	else if (source_samrate_ == 8000)
		sample_rate_index = 0x0b;

	PutBitContext pb;
	init_put_bits(&pb, pbuf, 1024);
	put_bits(&pb, 5, 2);    //object type - AAC-LC
	put_bits(&pb, 4, sample_rate_index); // sample rate index
	put_bits(&pb, 4, source_channel_);    // channel configuration
	//GASpecificConfig
	put_bits(&pb, 1, 0);    // frame length - 1024 samples
	put_bits(&pb, 1, 0);    // does not depend on core coder
	put_bits(&pb, 1, 0);    // is not extension

	flush_put_bits(&pb);

	pbuf += 2;

	return pbuf;
}

// 当收到sps和pps信息时，发送AVC和AAC的sequence header
void RtmpLiveEncoder::OnSPSAndPPS(char* spsBuf, int spsSize, char* ppsBuf, int ppsSize)
{
	sps_ = (char*)malloc(spsSize);
	memcpy(sps_, spsBuf, spsSize);
	sps_size_ = spsSize;

	pps_ = (char*)malloc(ppsSize);
	memcpy(pps_, ppsBuf, ppsSize);
	pps_size_ = ppsSize;

	if (is_enable_video_)
		SendAVCSequenceHeaderPacket();
	if (is_enable_audio_)
		SendAACSequenceHeaderPacket();

	m_pRtmpLiveQue->time_begin_ = ::GetTickCount();
	m_pRtmpLiveQue->last_timestamp_ = m_pRtmpLiveQue->time_begin_;

	s_VideoBegin = true;
}