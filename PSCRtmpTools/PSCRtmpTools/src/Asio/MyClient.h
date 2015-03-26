#ifndef MYCLIENT_H_
#define MYCLIENT_H_
#include <string>
#include <vector>

//boost asio head 
#ifdef _MSC_VER
#define _WIN32_WINNT 0x0501
#endif 
#define BOOST_REGEX_NO_LIB
#define BOOST_DATE_TIME_SOURCE
#define BOOST_SYSTEM_NO_LOB
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/locale/encoding.hpp>
#include "../tools/MySingleton.h"
#include <boost/asio/io_service.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include "../Data/GameBase.h"

#define MAX_RESERVE_SIZE 1024*8  //最大接收尺寸

class CMyClient : public MySingleton<CMyClient>
{
public:
	CMyClient();
// 	CMyClient(boost::asio::io_service & io, std::string strAppIP) :ios(&io),
// 		ep(boost::asio::ip::address::from_string(strAppIP), s_port){}
	static void RunAction();//开始执行
	//virtual void OnRead(const std::string & str);//读取到的文件
	bool OnSend(const std::string & str);//发送事件 去掉虚拟性质
public:
	static int s_port;//端口号
	static std::string s_ip;//ip
	void Init(boost::asio::io_service & io, std::string strAppIP);
	void release();
//-------------------------------------私有代码段-----------------------------------------------------------
protected:
	typedef boost::shared_ptr<boost::asio::ip::tcp::socket> sock_pt;//client 不需要accept，直接connect
	void Start();
	void ReadAction();//读取事件添加
	void Conn_handler(const boost::system::error_code & ec, sock_pt sock);//异步联接处理结果
	void Read_handler(const boost::system::error_code & ec, std::size_t bytes_transferred);
	static void ConnectServer();//连接服务器
private:
	boost::asio::io_service * ios = NULL;//io_server对象
	boost::asio::ip::tcp::endpoint ep;//TCP 端点
	static sock_pt m_sock_pt_server;//服务端连接套接字 智能指针 (主要用于发送数据到服务器)
	std::shared_ptr<CGameBase> m_pIGameBase;//游戏基类，用于处理数据发送功能
	char _Data[MAX_RESERVE_SIZE];
};

#endif