/********************************************************************
	创建时间：	2014/08/05  14:34:36
	文件名： 	MyIocp.h
	作者：		谢小军

	功能：		1. 用于服务器套接字连接
	2.

	说明：	    1.
	2.
*********************************************************************/

#ifndef MYIOCP_H_
#define MYIOCP_H_

#include <map>
#include <vector>
#include <string>
#include <winsock2.h>   
#pragma comment( lib, "ws2_32.lib" )   

#define MYIOCP_BUFFER_MAX_SIZE  4096
const int OP_READ = 0;
const int OP_WRITE = 1;
const int OP_ACCEPT = 2;

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

/*
	 OVERLAPPEDPLUS 结构体设计思路
	 OVERLAPPED 是一个固定的用于处理网络消息事件返回值的结构体变量
	 在完成端口和重叠I/O模型里用于返回消息事件的结果
	 因为在处理网络消息的时候，发送的是一个返回值的结构体指针，只要结构体
	 的前面部分满足系统的要求，在系统操作成功的时候也就会把这个结构体指针
	 发回给用户，我们只要在系统定义的结构体后面扩展一些自己的东西，就可以
	 很轻松的确定该消息是谁发过来的。
	 不过好像完成端口在设计的时候也满足了这样的需求，所以在这里我只是放入
	 一些与系统连接有关的数据，用户需要存放的数据这里就不在存放
	 这里存储与系统相关的数据有：
	 socket
	 OpCode 本次消息的操作类型（在完成端口的操作里面，是以消息通知系统，
	 读数据/写数据，都是要发这样的消息结构体过去的，所以如果系统要同时
	 进行读写操作的话，就需要有一个变量来区分操作了）

	 WSABUF   wbuf;                  //   读写缓冲区结构体变量
	 DWORD    dwBytes, dwFlags;		//   一些在读写时用到的标志性变量
	 char buf[4096];                //   自己的缓冲区
	 上面的4个变量存放的是一些与消息相关的数据，都是一些操作上用到的，
	 这些东西都是固定的，具体作用需要参考一下完成端口相关函数的参数接口
	 */
struct OVERLAPPEDPLUS
{
	OVERLAPPED    ol;
	SOCKET        s;
	int		OpCode;
	WSABUF   wbuf;
	DWORD    dwBytes, dwFlags;
	char buf[MYIOCP_BUFFER_MAX_SIZE];
};

class CMyIOCP
{
protected:
	HANDLE					g_hwThread;     //   工作线程句柄
	DWORD					m_wthreadID;
	HANDLE					g_haThread;     //   连接线程句柄
	DWORD					m_athreadID;
	CriticalSection			m_criticalSection;	//	 临界区
public:
	bool					m_workThread;
	bool					m_acceptThread;
	HANDLE					m_hIocp;             //   完成端口的句柄
	SOCKET					m_sSocket;
	std::map<SOCKET,SOCKET>	m_mapSocket;//用于保存连接的套接字，主要用于发送消息
	std::vector<std::string> m_verData;
public:
	CMyIOCP(void);
	CMyIOCP(int port){Listen(port);};
	~CMyIOCP(void);
	virtual void OnRead(void * p, char *buf, int len){ TRACE("读取数据接口"); };
	virtual void OnSend(void * p, char *buf, int len){ TRACE("写入数据接口"); };
	virtual void OnAccept(SOCKET socket);
	virtual void OnClose(void * p){TRACE("关闭socket");}; 

	void DeleteMapSocket(SOCKET socket);
	void DeleteAccepted();
	void DeleteListener();
public:
	bool SetIoCompletionPort(SOCKET socket, void *p, char *buf = NULL, int len = 0);
	//   把一个socket与一个自定义的结构体关联到完成端口（相当于把socket与一个结构体变量进行绑定），  
	//   这样当发送上面3种网络事件的时候，该结构体变量会再传回给程序   
	//   这样就可以区分当前网络事件是那个socket发出的 
	bool Init(void);
	bool Listen(int port);
	bool Send(SOCKET socket,const char *buf, int len);
	void SendAll(const char *buf, int len);
	static DWORD __stdcall WorkThread(LPVOID Param);
	static DWORD __stdcall AcceptThread(LPVOID Param);
	void DeleteIocp();//处理关闭事件
};

#endif
