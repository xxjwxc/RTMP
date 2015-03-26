#ifndef __RTMPLIVEQUE_H_
#define __RTMPLIVEQUE_H_
#include "..\tools\MySingleton.h"
#include "..\tools\Queue\MyQueue.h"
#include "base\DataBuffer.h"

//编码数据缓存
struct RtmpDataBuffer
{
	int type;
	base::DataBuffer* data;
	unsigned int timestamp;
	bool is_keyframe;

	RtmpDataBuffer(int type_i, base::DataBuffer* data_i, unsigned int ts_i, bool iskey_i)
	{
		type = type_i;
		data = data_i;
		timestamp = ts_i;
		is_keyframe = iskey_i;
	}

	RtmpDataBuffer()
	{
		type = -1;
		data = NULL;
		timestamp = 0;
		is_keyframe = false;
	}
};

class CRtmpLiveQue :public MySingleton<CRtmpLiveQue>
{
public:
	CRtmpLiveQue();
	~CRtmpLiveQue();
public:
	void OnCaptureAudioBuffer(base::DataBuffer* dataBuf, unsigned int timestamp);

	void OnCaptureVideoBuffer(base::DataBuffer* dataBuf,/* unsigned int timestamp,*/ bool isKeyframe);

	unsigned int GetTimestamp();

	__int64 time_begin_;
	__int64 last_timestamp_;

public://公共 线程 安全的
	CMyQueue<RtmpDataBuffer> m_que;
	//std::deque<RtmpDataBuffer> process_buf_queue_;
};

//播放控制
class EcoderCtr
{
public:
	EcoderCtr(){};
	~EcoderCtr(){};
	void Pause(){ m_is_pause = true; };//暂停
	void Start()
	{
		m_is_pause = false;
		if (m_is_stop)
			m_is_stop = false;
	}

	void Stop(){ m_is_stop = true; };//停止

	bool m_is_stop = true;
	bool m_is_pause = false;
};
#endif