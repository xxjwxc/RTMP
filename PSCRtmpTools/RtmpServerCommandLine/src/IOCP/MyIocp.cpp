//   Iocp 实现文件 
#include "StdAfx.h"  
#include "MyIocp.h"   

static bool g_bInit = false;

DWORD __stdcall CMyIOCP::WorkThread(LPVOID Param)
{
	CMyIOCP * pthis = (CMyIOCP *)Param;

	void * re;
	OVERLAPPED * pOverlap;
	DWORD berByte;
	while (pthis->m_workThread)
	{
		if (GetQueuedCompletionStatus(pthis->m_hIocp, &berByte, (LPDWORD)&re, (LPOVERLAPPED *)&pOverlap, INFINITE) == ERROR_SUCCESS)
		{
			//失败
			OVERLAPPEDPLUS *olp = (OVERLAPPEDPLUS *)pOverlap;
			if (olp) 
			{
				pthis->OnClose(re);
				pthis->DeleteMapSocket(olp->s);
				closesocket(olp->s);
				delete olp;        //   释放 与socket绑定的结构体变量 
			}

			continue;
		}

		if (berByte == 0)
		{
			//   客户端断开连接   
			if (re)
				pthis->OnClose(re);

			OVERLAPPEDPLUS *olp = (OVERLAPPEDPLUS *)pOverlap;
			if (olp)
			{
				pthis->DeleteMapSocket(olp->s);
				closesocket(olp->s);
				delete olp;        //   释放 与socket绑定的结构体变量 
			}
		}

		if (re == NULL) return 0;

		OVERLAPPEDPLUS *olp = (OVERLAPPEDPLUS *)pOverlap;

		switch (olp->OpCode)
		{
		case OP_READ://读取
			{
				pthis->OnRead(re, olp->wbuf.buf, berByte);     //   调用 OnRead() 通知应用程序，服务器收到来自客户端的网络数据   
				WSARecv(olp->s, &olp->wbuf, 1, &olp->dwBytes, &olp->dwFlags, &olp->ol, NULL); //   继续调用一个接收的 I/O 异步请求
				break;
			}

		case OP_WRITE://写完
			{
				pthis->OnSend(re,olp->wbuf.buf, berByte);
				delete olp; //释放 与socket绑定的结构体变量
				break;
			}
		default:
			break;
		}
	}
	return 0;
}

DWORD __stdcall CMyIOCP::AcceptThread(LPVOID Param)
{
	CMyIOCP * pthis = (CMyIOCP *)Param;
	while (pthis->m_acceptThread)
	{
		SOCKET client;
		if ((client = accept(pthis->m_sSocket, NULL, NULL)) != INVALID_SOCKET)
			pthis->OnAccept(client);    //   调用 OnAccept()通知应用程序有新客户端连接   
		else
			break;
	}
	return 1;
}

CMyIOCP::CMyIOCP()
{
	m_verData.clear();
	m_mapSocket.clear();
}

bool CMyIOCP::Init(void)
{
	if (g_bInit) return true;

	m_mapSocket.clear();

	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
		return false;

	g_bInit = true;
	return true;
}

bool CMyIOCP::Listen(int port)
{
	if (!g_bInit && !Init())
			return false;

	m_sSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_sSocket == INVALID_SOCKET)
		return false;

	//SOCKADDR_IN addr;    
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");    

	if (bind(m_sSocket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
		return false;

	if (listen(m_sSocket, 10) == SOCKET_ERROR)
		return false;

	if ((m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0)) == NULL)     //   创建完成端口的句柄
		return false;

	this->m_acceptThread = true;
	g_haThread = CreateThread(NULL, 0, AcceptThread, (LPVOID)this, 0, &m_athreadID);    //   创建连接线程，用来接收客户端的连接

	this->m_workThread = true;
	g_hwThread = CreateThread(NULL, 0, WorkThread, (LPVOID)this, 0, &m_wthreadID); //   创建工作线程，用来处理完成端口消息的

	return true;
}

void CMyIOCP::OnAccept(SOCKET socket)
{
	if(this->SetIoCompletionPort(socket, NULL))
	{
		Lock lock(&m_criticalSection);//获取临界区
		m_mapSocket[socket] = socket;
		if (m_verData.size() > 0)
		{
			for(int i=0;i<m_verData.size();i++)//发送先前部分
				Send(socket,m_verData.at(i).c_str(),m_verData.at(i).length());
		}
	}
}

void CMyIOCP::DeleteMapSocket(SOCKET socket)
{
	Lock lock(&m_criticalSection);//获取临界区
	m_mapSocket[socket] = 0;
}

bool CMyIOCP::SetIoCompletionPort(SOCKET socket, void *p, char *buf, int len)
{
	if (CreateIoCompletionPort((HANDLE)socket, m_hIocp, (unsigned long)socket, 0) == NULL)
		return false;

	OVERLAPPEDPLUS *olp = new OVERLAPPEDPLUS;
	memset(olp, 0, sizeof(OVERLAPPEDPLUS));
	olp->s = socket;
	if (buf)
	{
		//   这里可以使用用户自定义的缓冲区地址，如果用户不想设置，也可以采用默认分配的缓冲区   
		olp->wbuf.buf = buf;
		olp->wbuf.len = len;
	}
	else
	{
		olp->wbuf.buf = olp->buf;
		olp->wbuf.len = MYIOCP_BUFFER_MAX_SIZE;
	}

	olp->OpCode = OP_READ;
	int ret = WSARecv(olp->s, &olp->wbuf, 1, &olp->dwBytes, &olp->dwFlags, &olp->ol, NULL);
	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
			return false;

	return true;
}

bool CMyIOCP::Send(SOCKET socket,const char *buf, int len)
{
	OVERLAPPEDPLUS *olp = new OVERLAPPEDPLUS;
	memset(olp, 0, sizeof(OVERLAPPEDPLUS));
	olp->s = socket;
	olp->wbuf.buf = olp->buf;
	olp->wbuf.len = MYIOCP_BUFFER_MAX_SIZE;
	memcpy(olp->wbuf.buf,buf,len);

	olp->OpCode = OP_WRITE;

	if (SOCKET_ERROR == WSASend(olp->s, &olp->wbuf, 1, &olp->dwBytes, olp->dwFlags, &olp->ol, NULL))
	{
		int nErrCode = WSAGetLastError();
		if ( ERROR_IO_PENDING != nErrCode ) 
		{
			return false;
		}
	}

	return true;
}

void CMyIOCP::SendAll(const char *buf, int len)
{
	Lock lock(&m_criticalSection);//获取临界区

	//整理
	std::map<SOCKET,SOCKET>::iterator it = m_mapSocket.begin();
	while(it != m_mapSocket.end())
	{
		if(it->second ==0)
		{
			std::map<SOCKET,SOCKET>::iterator itTemp = it;
			itTemp++;
			m_mapSocket.erase(it);
			it = itTemp;
		}
		else
			it++;
	}

	if (m_mapSocket.size() == 0)
		m_verData.push_back(std::string(buf));
	else
		m_verData.clear();

	for (std::map<SOCKET,SOCKET>::const_iterator c_it = m_mapSocket.begin();c_it != m_mapSocket.end();c_it++)
	{
		for(int i=0;i<m_verData.size();i++)//发送先前部分
			Send(c_it->second,m_verData.at(i).c_str(),m_verData.at(i).length());

		Send(c_it->second,buf,len);
	}

}

void CMyIOCP::DeleteAccepted()
{
 	for (std::map<SOCKET,SOCKET>::const_iterator c_it = m_mapSocket.begin();c_it != m_mapSocket.end();c_it++)
 	{
		if (c_it->second != INVALID_SOCKET) 
	 		::closesocket(c_it->second);
 	}
}

void CMyIOCP::DeleteListener()
{
	if (m_sSocket != INVALID_SOCKET)
	{
		closesocket(m_sSocket);
		m_sSocket = INVALID_SOCKET;
	}
}

void CMyIOCP::DeleteIocp()//处理关闭事件
{
	{
		Lock lock(&m_criticalSection);//获取临界区
		DeleteListener();
 		//整理
 		std::map<SOCKET,SOCKET>::iterator it = m_mapSocket.begin();
 		while(it != m_mapSocket.end())
 		{
 			if(it->second ==0)
 			{
 				std::map<SOCKET,SOCKET>::iterator itTemp = it;
 				itTemp++;
 				m_mapSocket.erase(it);
 				it = itTemp;
 			}
 			else
 				it++;
 		}

		DeleteAccepted(); 
		m_mapSocket.clear();
		m_verData.clear();
	}


	PostQueuedCompletionStatus(m_hIocp, 0, (DWORD) NULL, NULL);
	this->m_acceptThread = false;
	this->m_workThread = false;
	g_bInit = false;

	//等待一个线程结束
	WaitForSingleObject(g_haThread,INFINITE);
	WaitForSingleObject(g_hwThread,INFINITE);
}

CMyIOCP::~CMyIOCP(void)
{
	DeleteIocp();
}