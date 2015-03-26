#include "StdAfx.h"
#include "MyServer.h"

DWORD __stdcall CMyServer::AcceptThread(LPVOID Param)
{
	CMyServer * pthis = (CMyServer *)Param;
	while (pthis->m_acceptThread)
	{
		SOCKET client;
		if ((client = accept(pthis->m_sSocket, NULL, NULL)) != INVALID_SOCKET)
		{
			pthis->OnAccept(client);    //   调用 OnAccept()通知应用程序有新客户端连接 
		}  
		else
			break;
	}

	pthis->m_acceptThread = false;
	return 1;
}

DWORD __stdcall CMyServer::ReadThread(LPVOID Param)
{
	CMyServer * pthis = (CMyServer *)Param;
	while (pthis->m_readThread)
	{
		char recvBuf[MYIOCP_BUFFER_MAX_SIZE];
		memset(recvBuf, 0, MYIOCP_BUFFER_MAX_SIZE*sizeof(char));
		int n = recv(pthis->m_ClientSocket, recvBuf, MYIOCP_BUFFER_MAX_SIZE - 1, 0);
		if (n <= 0)//已经关闭
		{
			Lock lock(&pthis->m_criticalSection);//获取临界区
			pthis->OnClose(NULL);
			break;
		}
		else
		{
			Lock lock(&pthis->m_criticalSection);//获取临界区
			pthis->OnRead(NULL, recvBuf, MYIOCP_BUFFER_MAX_SIZE);
		}
			
	}

	pthis->m_acceptThread = false;
	return 1;
}

CMyServer::CMyServer(void)
{
	m_sSocket = INVALID_SOCKET;
	m_verData.clear();
	m_port = -1;
	m_bInit = false;
	m_acceptThread = false;
	m_readThread = false;
	m_ClientSocket = INVALID_SOCKET;
}

CMyServer::~CMyServer(void)
{
	DeleteIocp();
}

void CMyServer::OnAccept(SOCKET socket)
{
	Lock lock(&m_criticalSection);//获取临界区
	m_ClientSocket = socket;
	if (m_verData.size() > 0)
	{
		for (int i = 0; i < m_verData.size(); i++)//发送先前部分
			Send(socket, m_verData.at(i).c_str(), m_verData.at(i).length());
	}

	m_readThread = true;
	g_hrThread = CreateThread(NULL, 0, ReadThread, (LPVOID)this, 0, &m_readthreadID);    //   创建连接线程，用来接收客户端的连接
}

bool CMyServer::Listen(int port, bool isMySelf)
{
	if (m_bInit)
		return false;

	m_acceptThread = true;
	m_port = -1;
	//第一步：加载socket库函数
	//**********************************************************
	WSADATA wsaData;
	int err;

	err = WSAStartup(MAKEWORD(1, 1), &wsaData);

	if (err != 0) {
		return 0;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return 0;
	}
	//**********************************************************

	//第二步创建套接字
	m_sSocket = socket(AF_INET, SOCK_STREAM, 0);

	//第三步：绑定套接字

	//获取地址结构
	SOCKADDR_IN addrSrv;

	if (isMySelf)
		addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//监听本地
	else
		addrSrv.sin_addr.S_un.S_addr =  htonl(INADDR_ANY); //所有地址
	//将IP地址指定为INADDR_ANY，允许套接字向任何分配给本地机器的IP地址发送或接收数据
	//htonl()将主机的无符号长整形数转换成网络字节顺序。

	addrSrv.sin_family = AF_INET;
	//sin_family 表示地址族，对于IP地址，sin_family成员将一直是AF_INET

	//addrSrv.sin_port = htons(port);
	//htons()将主机的无符号短整形数转换成网络字节顺序

	//轮询查找
	bool isIni = false;
	for (int i = port; i < 65535; i++)
	{
		addrSrv.sin_port = htons(port);
		if (bind(m_sSocket, (struct sockaddr *)&addrSrv, sizeof(addrSrv)) != SOCKET_ERROR)//成功
		{
			m_port = i;
			break;
		}
		else
		{
			TRACE("Listen error");
		}
	}
	if (m_port == -1)
		return false;

	//监听客户端
	if (listen(m_sSocket, 10) == SOCKET_ERROR)
		return false;


	m_ClientSocket = NULL;
	g_haThread = CreateThread(NULL, 0, AcceptThread, (LPVOID)this, 0, &m_athreadID);    //   创建连接线程，用来接收客户端的连接

	return true;
}

void CMyServer::SendAll(const char *buf, int len)
{
	Lock lock(&m_criticalSection);//获取临界区

	if (m_ClientSocket == INVALID_SOCKET)
		m_verData.push_back(std::string(buf));
	else
	{
		for (int i = 0; i < m_verData.size(); i++)//发送先前部分
			Send(m_ClientSocket, m_verData.at(i).c_str(), m_verData.at(i).length());

		Send(m_ClientSocket, buf, len);
		m_verData.clear();
	}
}

bool CMyServer::Send(SOCKET socket, const char *buf, int len)
{
	DWORD    dwBytes;
// 	if (SOCKET_ERROR == WSASend(socket, buf, 1, &dwBytes, 0, NULL, NULL))
	if (SOCKET_ERROR == send(socket,buf,len,0))
	{
		this->OnClose(NULL);
		return false;
	}

	return true;
}

void CMyServer::DeleteIocp()
{
	{
		Lock lock(&m_criticalSection);//获取临界区
		closesocket(m_sSocket);
		m_sSocket = INVALID_SOCKET;
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
		m_verData.clear();
	}

	this->m_acceptThread = false;
	this->m_readThread = false;
	this->m_bInit = false;

	//等待一个线程结束
	WaitForSingleObject(g_haThread, INFINITE);
	WaitForSingleObject(g_hrThread, INFINITE);
}

std::string CMyServer::GetStrPort(std::string strHead)
{
	std::string str = strHead;
	char cs[7];
	memset(cs, 0, 7);
	sprintf_s(cs, "%d", m_port);
	str.append(cs);
	return str;
}
