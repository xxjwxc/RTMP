#ifndef _RTMP_LIVE_ENCODER_H_
#define _RTMP_LIVE_ENCODER_H_

#include "..\librtmp\LibRtmp.h"
#include "RtmpLiveQue.h"

class RtmpLiveEncoder:public EcoderCtr
{
public:
	RtmpLiveEncoder();

    ~RtmpLiveEncoder();

	void Init(int width, int height);//初始化长宽

    void StartLive();

    void StopLive();

    void Run();

//     virtual void OnCaptureAudioBuffer(base::DataBuffer* dataBuf, unsigned int timestamp);
// 
//     virtual void OnCaptureVideoBuffer(base::DataBuffer* dataBuf, unsigned int timestamp, bool isKeyframe);

  //  void PostBuffer(base::DataBuffer* dataBuf);

	// 当收到sps和pps信息时，发送AVC和AAC的sequence header
    void OnSPSAndPPS(char* spsBuf, int spsSize, char* ppsBuf, int ppsSize);//////

private:
	//发送视频数据包
    void SendVideoDataPacket(base::DataBuffer* dataBuf, unsigned int timestamp, bool isKeyframe);//////

	//发送音频数据包
    void SendAudioDataPacket(base::DataBuffer* dataBuf, unsigned int timestamp);//1111

    void SendMetadataPacket();/////

    void SendAVCSequenceHeaderPacket();/////

    void SendAACSequenceHeaderPacket();///......

    char* WriteMetadata(char* buf);//////

    char* WriteAVCSequenceHeader(char* buf);//////

    char* WriteAACSequenceHeader(char* buf);//......

public:
	std::shared_ptr<LibRtmp> m_libRtmp = nullptr;
	std::shared_ptr<CRtmpLiveQue> m_pRtmpLiveQue;
	static bool s_VideoBegin;
private:
	int source_samrate_;
	int source_channel_;

	char* sps_;        // sequence parameter set
	int sps_size_;
	char* pps_;        // picture parameter set
	int pps_size_;

	char* audio_mem_buf_;
	int audio_mem_buf_size_;
	char* video_mem_buf_;
	int video_mem_buf_size_;

	bool is_enable_audio_ = false;
	bool is_enable_video_ = false;

	// metadata
	int m_width = 0;
	int m_height = 0;


};

#endif // _RTMP_LIVE_ENCODER_H_
