/********************************************************************
创建时间：	2014/12/27  14:31:36
文件名： 	MyDescript.h
作者：		谢小军

功能：		1. 服务器与客户端通信
			2.用于具体逻辑处理

说明：	    1.
2.
*********************************************************************/
#ifndef  __MYDESCRIPT_H_
#define  __MYDESCRIPT_H_
#include "IOCP\MyServer.h"

class CmyDescript : public CMyServer
{
public:
	CmyDescript(int port = 1234,bool isLesonMySelf = false);
	~CmyDescript();
	void StartRun();//开始执行
public://接口回调
	virtual void OnRead(void * p, char *buf, int len);//{ TRACE("读取数据接口"); };
	//virtual void OnSend(void * p, char *buf, int len);// { TRACE("写入数据接口"); }; 禁止读取
private:
	int m_port = 0;//监听端口号
	bool m_isLesonMySelf = false;//是否本地监听
};
#endif