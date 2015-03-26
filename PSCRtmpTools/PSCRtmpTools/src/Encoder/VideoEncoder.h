#ifndef _VIDEO_ENCODER_THREAD_H_
#define _VIDEO_ENCODER_THREAD_H_

#include <windef.h>
#include <string>
#include "X264Encoder.h"
#include "AudioEncoder.h"
#include <atlimage.h>
#include "RtmpLiveEncoder.h"


class VideoEncoderThread:public EcoderCtr
{
public:
	void Init(int w, int h, RtmpLiveEncoder * _live);

	VideoEncoderThread();

    ~VideoEncoderThread();

    void Run();

private:

    bool RGB2YUV420(LPBYTE RgbBuf,UINT nWidth,UINT nHeight,LPBYTE yuvBuf,unsigned long *len);

    bool RGB2YUV420_r(LPBYTE RgbBuf,UINT nWidth,UINT nHeight,LPBYTE yuvBuf,unsigned long *len);

    void SaveRgb2Bmp(char* rgbbuf, unsigned int width, unsigned int height);

	bool Bmp24ToYuv(LPBYTE image, UINT nWidth, UINT nHeight, LPBYTE dstImage, unsigned long *len);

private:
    //DSVideoGraph* ds_video_graph_;
    X264Encoder* x264_encoder_;
    std::string filename_264_;
    //bool is_save_picture_;
	bool m_isShowMouse;
    //DSCaptureListener* listener_;
	int m_width = 0;
	int m_height = 0;
	std::shared_ptr<CRtmpLiveQue> m_pRtmpLiveQue;
	RtmpLiveEncoder * m_live;
	static int g_fps;
};

#endif // _VIDEO_ENCODER_THREAD_H_
