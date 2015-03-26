/********************************************************************
	创建时间：	2014/08/15  16:26:54 
	文件名：		MyQueue.h
	作者：		谢小军
	
	功能：		1. 线程安全队列
				2.
				
	说明：	    1. 多线程状态下的安全队列
				2. 
*********************************************************************/
#ifndef MYQUEUE_H_
#define MYQUEUE_H_
#include <queue>//std queue
#include <boost/thread/mutex.hpp>

template<typename TYPE>
class CMyQueue
{
private:
	std::queue<TYPE> theQueue_;
	boost::mutex m_oMutex;
public:
	CMyQueue(){};
	~CMyQueue()
	{
		boost::mutex::scoped_lock oLock(m_oMutex);//互斥锁
		while(!theQueue_.empty())
		{
			TYPE tmp = theQueue_.front();
			if (tmp.data)
			{
				delete tmp.data;
			}
			theQueue_.pop();
		}
	};
public:
	TYPE pop();//出一个队列
	void push( const TYPE &val );//压入一个队列
	size_t size();//队列长度
	bool empty();//判空
};
template<typename TYPE>
TYPE CMyQueue<TYPE>::pop()
{
	boost::mutex::scoped_lock oLock(m_oMutex);//互斥锁
	if (!theQueue_.empty())
	{
		TYPE tmp = theQueue_.front();
		theQueue_.pop();
		return tmp;
	}
	else
		return TYPE();//空的返回默认
}

template<typename TYPE>
void CMyQueue<TYPE>::push( const TYPE &val )
{
	boost::mutex::scoped_lock oLock(m_oMutex);//互斥锁
	theQueue_.push(val);
}

template<typename TYPE>
size_t CMyQueue<TYPE>::size()
{
	//boost::mutex::scoped_lock oLock(m_oMutex);//互斥锁
	//	size_t size=theQueue_.size();
	return theQueue_.size();
}

template<typename TYPE>
bool CMyQueue<TYPE>::empty()
{
	boost::mutex::scoped_lock oLock(m_oMutex);//互斥锁
	return theQueue_.empty();
}

#endif