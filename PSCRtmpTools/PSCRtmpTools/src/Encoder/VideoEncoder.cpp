#include "stdafx.h"
#include "VideoEncoder.h"
#include <wingdi.h>
#include <afxwin.h>
#include "..\tools\MyIni.h"
#include <boost\progress.hpp>

int VideoEncoderThread::g_fps = 25;

VideoEncoderThread::VideoEncoderThread() : x264_encoder_(new X264Encoder),m_live(nullptr)
//	, is_save_picture_(false)
{
	CMyIni m_ini;

	g_fps = m_ini.GetIntConfig("FPS");
	m_isShowMouse = m_ini.GetIntConfig("isShowMouse");
}

VideoEncoderThread::~VideoEncoderThread()
{
	//x264_encoder_->Destroy();
	delete x264_encoder_;
}

void VideoEncoderThread::Init(int w, int h, RtmpLiveEncoder * _live)
{
	m_width = w;
	m_height = h;
	m_live = _live;

	m_pRtmpLiveQue = CRtmpLiveQue::instance();

	// 初始化工作
	x264_encoder_->Initialize(w, h, 96, g_fps);
}

#define YUNINBUF 1
void VideoEncoderThread::Run()
{
	unsigned long yuvimg_size = m_width * m_height * 3 / 2;
	unsigned char* yuvbuf = (unsigned char*)malloc(yuvimg_size);
	unsigned char* x264buf = (unsigned char*)malloc(yuvimg_size * YUNINBUF);
	unsigned char* rgbbuf = new unsigned char[m_width * m_height * 4];

	unsigned char* _f = nullptr,*_e = nullptr;

	//__int64 last_tick = 0;
	unsigned int timestamp = 0;
	unsigned int last_idr_timestamp = 0;

	int x264buf_len = 0;
	bool is_first = true;

	double cap_interval = 1.0 / g_fps;
	double dl = 0.0f,_dl=0.0f;
	//unsigned int now_tick = 0, next_tick = 0;

	boost::timer time_elapse;//流逝时间
	//循环依次查找
	while (!m_is_stop)
	{
		if (m_is_pause)
		{
			Sleep(1000);
			continue;
		}
		time_elapse.restart();//重新开始计算

		//------------------------------------截取屏幕
		CDC *pDC;//屏幕DC
		pDC = CDC::FromHandle(::GetDC(NULL));//获取当前整个屏幕DC

		CDC memDC;//内存DC
		memDC.CreateCompatibleDC(pDC);

		CBitmap memBitmap, *oldmemBitmap;//建立和屏幕兼容的bitmap
		memBitmap.CreateCompatibleBitmap(pDC, m_width, m_height);

		oldmemBitmap = memDC.SelectObject(&memBitmap);//将memBitmap选入内存DC
		memDC.BitBlt(0, 0, m_width, m_height, pDC, 0, 0, SRCCOPY);//复制屏幕图像到内存DC

		if (m_isShowMouse)
		{
			//添加鼠标到图片里
			POINT l_pt;
			::GetCursorPos(&l_pt);
			HCURSOR l_hCursor = ::GetCursor();
			::DrawIcon(memDC.GetSafeHdc(), l_pt.x, l_pt.y, l_hCursor);
		}

		memBitmap.GetBitmapBits(m_width*m_height*4, rgbbuf);//复制图形数据

		//BITMAP bmp;
		//memBitmap.GetBitmap(&bmp);
		//
		//int nChannels;
		//if (bmp.bmBitsPixel == 1)//得到图像深度和通道数 
		//	nChannels = 1;
		//else
		//	nChannels = bmp.bmBitsPixel / 8;

		//memBitmap.GetBitmapBits(bmp.bmHeight*bmp.bmWidth*nChannels, rgbbuf);//复制图形数据
		
// 		_f = rgbbuf;
// 		for (int i = 0; i < bmp.bmHeight; i++)
// 		{
// 			for (int j = 0; j < bmp.bmWidth; j++)
// 			{
// 				*_f = rgbbuf[i*bmp.bmWidthBytes + j * 4];//b
// 				*(_f + 1) = rgbbuf[i*bmp.bmWidthBytes + j * 4 + 1];//g
// 				*(_f + 2) = rgbbuf[i*bmp.bmWidthBytes + j * 4 + 2];//r
// 				_f += 3;
// 			}
// 		}

		memDC.SelectObject(oldmemBitmap);
		memDC.DeleteDC();
		memBitmap.DeleteObject(); 
		oldmemBitmap->DeleteObject();
		pDC->DeleteDC();

//-----------------------------------------
		_dl += dl;
		bool is_keyframe = false;
		if (_dl >= 3.0)
		{
			is_keyframe = true;
			_dl = 0.0;
		}
		
		Bmp24ToYuv((LPBYTE)rgbbuf, m_width, m_height, (LPBYTE)yuvbuf, &yuvimg_size);
		//RGB2YUV420_r((LPBYTE)rgbbuf, m_width, m_height, (LPBYTE)yuvbuf, &yuvimg_size);

		x264buf_len = yuvimg_size * YUNINBUF;
		x264_encoder_->Encode(yuvbuf, x264buf, x264buf_len, is_keyframe);
		if (is_first)
		{
			m_live->OnSPSAndPPS(x264_encoder_->SPS(), x264_encoder_->SPSSize(), x264_encoder_->PPS(), x264_encoder_->PPSSize());
			is_first = false;
		}

		if (x264buf_len > 0)
		{
			if (m_pRtmpLiveQue && x264buf_len)
			{
				base::DataBuffer* dataBuf = new base::DataBuffer((char*)x264buf, x264buf_len);
				m_pRtmpLiveQue->OnCaptureVideoBuffer(dataBuf, is_keyframe);
			}
		}


		//now_tick = ::GetTickCount();
		dl = time_elapse.elapsed();
		if (cap_interval > dl)
		{
			::Sleep((cap_interval - dl)*1000);
		}
	}

	delete[] rgbbuf;
	free(yuvbuf);
	free(x264buf);
	
}

bool VideoEncoderThread::RGB2YUV420(LPBYTE RgbBuf, UINT nWidth, UINT nHeight, LPBYTE yuvBuf, unsigned long *len)
{
	int i, j;
	unsigned char*bufY, *bufU, *bufV, *bufRGB, *bufYuv;
	//memset(yuvBuf,0,(unsigned int )*len);
	bufY = yuvBuf;
	bufV = yuvBuf + nWidth * nHeight;
	bufU = bufV + (nWidth * nHeight * 1 / 4);
	*len = 0;
	unsigned char y, u, v, r, g, b, testu, testv;
	unsigned int ylen = nWidth * nHeight;
	unsigned int ulen = (nWidth * nHeight) / 4;
	unsigned int vlen = (nWidth * nHeight) / 4;
	for (j = 0; j < nHeight; j++)
	{
		bufRGB = RgbBuf + nWidth * (nHeight - 1 - j) * 3;
		for (i = 0; i < nWidth; i++)
		{
			int pos = nWidth * i + j;
			r = *(bufRGB++);
			g = *(bufRGB++);
			b = *(bufRGB++);

			y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
			u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
			v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
			*(bufY++) = max(0, min(y, 255));

			if (j % 2 == 0 && i % 2 == 0)
			{
				if (u > 255)
				{
					u = 255;
				}
				if (u < 0)
				{
					u = 0;
				}
				*(bufU++) = u;
				//存u分量
			}
			else
			{
				//存v分量
				if (i % 2 == 0)
				{
					if (v > 255)
					{
						v = 255;
					}
					if (v < 0)
					{
						v = 0;
					}
					*(bufV++) = v;
				}
			}
		}

	}
	*len = nWidth * nHeight + (nWidth * nHeight) / 2;
	return true;
}

bool VideoEncoderThread::RGB2YUV420_r(LPBYTE RgbBuf, UINT nWidth, UINT nHeight, LPBYTE yuvBuf, unsigned long *len)
{
	int i, j;
	unsigned char*bufY, *bufU, *bufV, *bufRGB;
	//memset(yuvBuf,0,(unsigned int )*len);
	bufY = yuvBuf;
	bufV = yuvBuf + nWidth * nHeight;
	bufU = bufV + (nWidth * nHeight * 1 / 4);
	*len = 0;
	unsigned int y, u, v, r, g, b;

	for (j = nHeight - 1; j >= 0; j--)
	{
		bufRGB = RgbBuf + nWidth * (nHeight - 1 - j) * 3;
		for (i = nWidth - 1; i >= 0; i--)
		{
			//unsigned char* prgb = bufRGB + i*3;
			//int pos = nWidth * i + j;
			b = *(bufRGB++);
			g = *(bufRGB++);
			r = *(bufRGB++);

			y = (65 * r + 129 * g + 25 * b + ((r + b) >> 1) + 25 + 4096)>>8; //or 25 
			u = (-38 * r - 74 * g + 112 * b + 128 + 32768 + 128)>>8;
			v = (112 * r - 94 * g - 18 * b + 128 + 32768 + 128)>>8;
// 			y = (unsigned int)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
// 			u = (unsigned int)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
// 			v = (unsigned int)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

			*(bufY++) = max(0, min((unsigned char)y, 255));

			if (j % 2 == 0 && i % 2 == 0)
			{
				if (u > 255)
					u = 255;
				if (u < 0)
					u = 0;

				*(bufU++) = (unsigned char)u;
				//存u分量
			}
			else
			{
				//存v分量
				if (i % 2 == 0)
				{
					if (v > 255)
						v = 255;
					if (v < 0)
						v = 0;

					*(bufV++) = (unsigned char)v;
				}
			}

		}

	}
	*len = nWidth * nHeight + (nWidth * nHeight) / 2;
	return true;
}

#pragma warning(disable:4146)
void VideoEncoderThread::SaveRgb2Bmp(char* rgbbuf, unsigned int width, unsigned int height)
{
	BITMAPINFO bitmapinfo;
	ZeroMemory(&bitmapinfo, sizeof(BITMAPINFO));
	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biWidth = width;
	bitmapinfo.bmiHeader.biHeight = (-height);
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 24;
	bitmapinfo.bmiHeader.biXPelsPerMeter = 0;
	bitmapinfo.bmiHeader.biYPelsPerMeter = 0;
	bitmapinfo.bmiHeader.biSizeImage = width*height;
	bitmapinfo.bmiHeader.biClrUsed = 0;
	bitmapinfo.bmiHeader.biClrImportant = 0;

	BITMAPFILEHEADER bmpHeader;
	ZeroMemory(&bmpHeader, sizeof(BITMAPFILEHEADER));
	bmpHeader.bfType = 0x4D42;
	bmpHeader.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
	bmpHeader.bfSize = bmpHeader.bfOffBits + width*height * 3;

	FILE* fp = NULL;
	fopen_s(&fp, "./CameraCodingCapture.bmp", "wb");
	if (fp)
	{
		fwrite(&bmpHeader, 1, sizeof(BITMAPFILEHEADER), fp);
		fwrite(&(bitmapinfo.bmiHeader), 1, sizeof(BITMAPINFOHEADER), fp);
		fwrite(rgbbuf, 1, width*height * 3, fp);
		fclose(fp);
	}
}

bool VideoEncoderThread::Bmp24ToYuv(LPBYTE BmpBuf, UINT nWidth, UINT nHeight, LPBYTE yuvBuf, unsigned long *len)
{
	int i, j;
	unsigned char*bufY, *bufU, *bufV, *bufRGB;
	//memset(yuvBuf,0,(unsigned int )*len);
	bufY = yuvBuf;
	bufV = yuvBuf + nWidth * nHeight;
	bufU = bufV + (nWidth * nHeight * 1 / 4);
	*len = 0;
	unsigned int y, u, v, r, g, b;

	for (j = nHeight - 1; j >= 0; j--)
	{
		bufRGB = BmpBuf + nWidth * (nHeight - 1 - j) * 4;
		for (i = nWidth - 1; i >= 0; i--)
		{
			//unsigned char* prgb = bufRGB + i*3;
			//int pos = nWidth * i + j;
			r = *(bufRGB++);
			g = *(bufRGB++);
			b = *(bufRGB++);
			bufRGB ++ ;

			y = (65 * r + 129 * g + 25 * b + ((r + b) >> 1) + 25 + 4096) >> 8; //or 25 
			u = (-38 * r - 74 * g + 112 * b + 128 + 32768 + 128) >> 8;
			v = (112 * r - 94 * g - 18 * b + 128 + 32768 + 128) >> 8;
// 			y = (unsigned int)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
// 			u = (unsigned int)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
// 			v = (unsigned int)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

			*(bufY++) = max(0, min((unsigned char)y, 255));

			if (j % 2 == 0 && i % 2 == 0)
			{
				if (u > 255)
					u = 255;
				if (u < 0)
					u = 0;

				*(bufU++) = (unsigned char)u;
				//存u分量
			}
			else
			{
				//存v分量
				if (i % 2 == 0)
				{
					if (v > 255)
						v = 255;
					if (v < 0)
						v = 0;

					*(bufV++) = (unsigned char)v;
				}
			}

		}

	}
	*len = nWidth * nHeight + (nWidth * nHeight) / 2;
	return true;
}