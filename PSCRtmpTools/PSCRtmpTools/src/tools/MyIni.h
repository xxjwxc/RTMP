/********************************************************************
	创建时间：	2014/04/13 10: 01: 43
	文件名：		FileInfo.h
	作者：		谢小军
	
	功能：		1. 操作配置文件类
				2.
				
	说明：	    1. 
				2. 
*********************************************************************/
#ifndef MYINI_H_
#define MYINI_H_
#include <string>

class CMyIni
{
public:
	static std::string GetStringConfig(const char * tag);
	static void SetStringConfig(const char * tag,std::string str);

	static int GetIntConfig(const char * tag);
	static void SetIntConfig(const char * tag,int n);

};

#endif