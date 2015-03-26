#include "stdafx.h"
#include "MyClient.h"
#include <boost/thread.hpp>

boost::asio::io_service g_ios;//asio 必须的 io_server对象 (即任务队列)
int CMyClient::s_port = 1234;
std::string CMyClient::s_ip = "127.0.0.1";
CMyClient::sock_pt CMyClient::m_sock_pt_server = NULL;
DECLARE_SINGLETON_MEMBER(CMyClient);

CMyClient::CMyClient()
{
	memset(_Data, 0, sizeof(_Data));
	m_pIGameBase = CGameBase::instance();
}

void CMyClient::RunAction()
{
	boost::thread thrd(&CMyClient::ConnectServer);
}

//连接服务器
void CMyClient::ConnectServer()
{
	std::shared_ptr<CMyClient> pMyClient = instance();
	pMyClient->Init(g_ios, s_ip);
	try
	{
		pMyClient->Start();
		g_ios.run();
		m_sock_pt_server.reset();
	}
	catch (std::exception & e)
	{

	}
}

bool CMyClient::OnSend(const std::string & str)
{
	if (m_sock_pt_server)
	{
		m_sock_pt_server->send(boost::asio::buffer(str));
		return true;
	}
	else
		return false;
}

void CMyClient::Start()
{
	sock_pt sock(new boost::asio::ip::tcp::socket(*ios));//创建socket 对象 智能指针，以便在异步调用过程中传递

	sock->async_connect(ep,//异步连接
		boost::bind(&CMyClient::Conn_handler, this, boost::asio::placeholders::error, sock));
}

void CMyClient::Conn_handler(const boost::system::error_code & ec, sock_pt sock)//异步联接处理结果
{
	if (ec)
	{
		Start();//一直轮询连接
		return;
	}

	if (m_pIGameBase)
	{
		m_pIGameBase->OnConnect();
		m_sock_pt_server = sock;
		ReadAction();
	}
}


void CMyClient::ReadAction()
{
	/*boost::shared_ptr<std::vector<char> > str(new std::vector<char>(MAX_RESERVE_SIZE, 0));*/
	m_sock_pt_server->async_read_some(boost::asio::buffer(_Data), boost::bind(&CMyClient::Read_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}


void CMyClient::Read_handler(const boost::system::error_code & ec, std::size_t bytes_transferred)
{
	if (ec)
	{
		if (m_pIGameBase)
		{
			m_sock_pt_server.reset();
			m_sock_pt_server = NULL;
			m_pIGameBase->DisConnect();
			Start();//一直轮询连接
		}

		return;
	}

	if (_Data[0] != '\0')
	{
		std::string str(_Data);
		if (m_pIGameBase)
			m_pIGameBase->OnRead(str);
	}

	//继续处理读取事件
	ReadAction();
};

void CMyClient::Init(boost::asio::io_service & io, std::string strAppIP)
{
	ios = &io;
	ep =boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(strAppIP), s_port);
}

void CMyClient::release()
{
	g_ios.stop();
	g_ios.reset();
	m_pIGameBase->Release();//调用删除接口
	m_pIGameBase = NULL;
	m_sock_pt_server.reset();
}
