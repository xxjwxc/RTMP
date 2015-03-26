#include "stdafx.h"
#include <afxwin.h>
#include "MyMainEcoder.h"
#include "..\tools\MyIni.h"
#include "..\Data\GameBase.h"

CMyMainEcode::CMyMainEcode()
{

}

CMyMainEcode::~CMyMainEcode()
{

}

void CMyMainEcode::Init()
{
	CMyIni m_ini;

	double _x = m_ini.GetIntConfig("percentW")*0.01;
	double _y = m_ini.GetIntConfig("percentH")*0.01;
	if (_x < 0.00005F || _x>1.0f)
		_x = 1.0f;
	if (_y < 0.00005F || _y>1.0f)
		_y = 1.0f;

	CDC *pDC = CDC::FromHandle(::GetDC(NULL));//获取当前整个屏幕DC
	int Width = (pDC->GetDeviceCaps(HORZRES)-1)*_x;
	int Height = (pDC->GetDeviceCaps(VERTRES)-1)*_y;
	//Width = 2048;
	//Height = 1024;
	if (Width % 2) Width -= 1;
	if (Height % 2) Height -= 1;
	m_RtmpLiveEcoder.Init(Width, Height);//初始化高宽

	m_videoEcoder.Init(Width, Height,&m_RtmpLiveEcoder);
	m_audioEcoder.Init();

}

void CMyMainEcode::Start()
{
	std::shared_ptr<CGameBase>  pGame = CGameBase::instance();
	if (pGame)
		pGame->OnSend("Start");

	if (m_is_pause == true)
	{
		m_RtmpLiveEcoder.Start();
		m_videoEcoder.Start();
		m_audioEcoder.Start();
		EcoderCtr::Start();
	}
	else if (m_is_stop)
	{
		EcoderCtr::Start();
		m_is_stop = false;

		m_RtmpLiveEcoder.Start();
		m_thread_rtmpLiveEcode = std::thread(std::bind(&RtmpLiveEncoder::Run,&m_RtmpLiveEcoder));
		::SetThreadPriority(m_thread_rtmpLiveEcode.native_handle(),THREAD_PRIORITY_HIGHEST);//设置最高优先级

		m_videoEcoder.Start();
		m_thread_video = std::thread(std::bind(&VideoEncoderThread::Run, &m_videoEcoder));

		m_audioEcoder.Start();
		m_thread_audio = std::thread(std::bind(&AudioEncoderThread::Run, &m_audioEcoder));
	}
}

void CMyMainEcode::Pause()
{
// 	std::shared_ptr<CGameBase>  pGame = CGameBase::instance();
// 	if (pGame)
// 		pGame->OnSend("Pause");

	EcoderCtr::Pause();
	//m_RtmpLiveEcoder.Pause();
	m_videoEcoder.Pause();
	m_audioEcoder.Pause();

}

void CMyMainEcode::Stop()
{
// 	std::shared_ptr<CGameBase>  pGame = CGameBase::instance();
// 	if (pGame)
// 		pGame->OnSend("Stop");

	if (!m_is_stop)
	{
		EcoderCtr::Stop();

		m_audioEcoder.Stop();
		m_thread_audio.join();

		m_videoEcoder.Stop();
		m_thread_video.join();

		m_RtmpLiveEcoder.Stop();
		m_thread_rtmpLiveEcode.join();
		m_is_stop = true;
	}
}

void CMyMainEcode::Refresh()
{
	std::shared_ptr<CGameBase>  pGame = CGameBase::instance();
	if (pGame) 
		pGame->OnSend("Refresh");
}



