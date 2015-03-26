#ifndef __MYMAINECODER_H_
#define __MYMAINECODER_H_
#include "RtmpLiveEncoder.h"
#include "VideoEncoder.h"
#include "AudioEncoder.h"
#include <thread>

class CMyMainEcode : public EcoderCtr
{
public:
	std::shared_ptr<LibRtmp> GetLiveEcode(){ return m_RtmpLiveEcoder.m_libRtmp; };
	void Init();//初始化 必须先初始化？
	void Start();//开始
	void Pause();//暂停
	void Stop();//停止
	void Refresh();//刷新

	CMyMainEcode();
	~CMyMainEcode();
private:
	RtmpLiveEncoder m_RtmpLiveEcoder;//与rtmp server通讯主类
	VideoEncoderThread m_videoEcoder;//视频编码主类
	AudioEncoderThread m_audioEcoder;//音频编码主类

// 	bool m_is_stop = true;
// 	bool m_is_pause = false;

	//线程
	std::thread m_thread_video;//视频编码线程
	std::thread m_thread_audio;//音频编码线程
	std::thread m_thread_rtmpLiveEcode;//rtmp server 通讯控制线程
};

#endif