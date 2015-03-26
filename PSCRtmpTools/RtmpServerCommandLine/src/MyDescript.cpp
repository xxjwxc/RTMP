#include "stdafx.h"
#include "MyDescript.h"


CmyDescript::CmyDescript(int port /*= 1234*/, bool isLesonMySelf /*= false*/) :
m_port(port), m_isLesonMySelf(isLesonMySelf)
{

}

CmyDescript::~CmyDescript()
{

}

void CmyDescript::StartRun()
{
	Listen(m_port, m_isLesonMySelf);

	TRACE("strat\n");
	//等待一个线程结束
	WaitForSingleObject(g_haThread, INFINITE);
	WaitForSingleObject(g_hrThread, INFINITE);
}

void CmyDescript::OnRead(void * p, char *buf, int len)
{
	TRACE((const char *)buf);
	TRACE("\nrefresh\n");
	ShellExecute(NULL, "open", "C:/Program Files/GRETECH/GomPlayer/GOM.exe", "-f rtmp://127.0.0.1/live/test", NULL, SW_SHOWNORMAL);
}
