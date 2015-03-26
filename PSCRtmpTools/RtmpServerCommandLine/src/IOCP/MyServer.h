/********************************************************************
	创建时间：	2014/08/05  14:34:36
	文件名： 	MyServer.h
	作者：		谢小军

	功能：		1. 用于服务器套接字连接
	2.

	说明：	    1.
	2.
*********************************************************************/

#ifndef __MYSERVER_H_
#define __MYSERVER_H_
#include <vector>
#include <string>
#include <winsock2.h>   
#pragma comment( lib, "ws2_32.lib" )   

#ifndef _MFC_VER
#define TRACE(ver) printf(ver); 
#endif

#define MYIOCP_BUFFER_MAX_SIZE  4096

class CriticalSection//临界区
{
public:
	CriticalSection(){InitializeCriticalSection(&m_CritiSect);};//初始化临界区
	~CriticalSection(){DeleteCriticalSection(&m_CritiSect);};//删除临界区
	void Enter(){EnterCriticalSection(&m_CritiSect);};//进入临界区
	void Leave(){LeaveCriticalSection(&m_CritiSect);};//离开临界区
private:
	CRITICAL_SECTION m_CritiSect;//真正临界区定义
};

class Lock//自定义锁
{
public:
	explicit Lock(CriticalSection * PCritSect){//不允许隐式转换
		m_pCritical=PCritSect;
		m_pCritical->Enter();
	}
	~Lock(){m_pCritical->Leave();}
private:
	CriticalSection * m_pCritical;//自定义临界区
};

class CMyServer
{
public:
	CMyServer(void);
	CMyServer(int port){ Listen(port,true); };
	~CMyServer(void);

	static DWORD __stdcall AcceptThread(LPVOID Param);
	static DWORD __stdcall ReadThread(LPVOID Param);
public:
	bool					m_acceptThread;
	bool					m_readThread;
	CriticalSection			m_criticalSection;	//	 临界区
	SOCKET					m_sSocket;
	SOCKET					m_ClientSocket;//用于保存连接的套接字，主要用于发送消息
	std::vector<std::string> m_verData;
	int						 m_port;
	bool					m_bInit;//是否初始化
	DWORD					m_athreadID;
	DWORD					m_readthreadID;
	HANDLE					g_haThread;     //   连接线程句柄
	HANDLE					g_hrThread;     //   连接线程句柄
	bool Listen(int port,bool isMySelf);//监听开始 port 最小端口号
	bool Send(SOCKET socket, const char *buf, int len);
	void SendAll(const char *buf, int len);
	void DeleteIocp();//处理关闭事件
	std::string GetStrPort(std::string strHead);
public:
	virtual void OnAccept(SOCKET socket);
	virtual void OnRead(void * p, char *buf, int len){ TRACE("读取数据接口\n"); };
	virtual void OnSend(void * p, char *buf, int len){ TRACE("写入数据接口\n"); };
	virtual void OnClose(void * p){ m_ClientSocket = NULL; TRACE("关闭socket\n"); };
};

#endif
