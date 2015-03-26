/********************************************************************
	创建时间：	2014/04/13 10: 01: 43
	文件名：		FileInfo.h
	作者：		谢小军
	
	功能：		1. 工具类
				2.
				
	说明：	    1. 
				2. 
*********************************************************************/
#ifndef MYTOOLS_H_
#define MYTOOLS_H_
#include <boost\algorithm\string.hpp>
#include <vector>

#define   FOLDER_NAME  "TXT"

//全局函数 初始化 所有
class MyTools
{
public:
	/*
	  获取自己模块路径
	*/
	static CString GetSelfModulePath();
public:
  /*
  fname:当前可执行路径里文件夹下data目录下文件名
  prints：要打印的值，一个元素（std::string）代表要打印的一行数据
  isClear:是否清除数据
  strFl: 相对文件夹
  */ 
  static void PrintToFile(std::string fname,std::vector<std::string> & prints,bool isClear=true, std::string strF = FOLDER_NAME);//打印结果

  /*
  fname: 当前可执行路径里文件夹下data目录下文件名
  reads：读出存储的位置，一行文件内容存到一个boost::string里
  strFl: 相对文件夹
  */
  static bool ReadFromFile(std::string fname,std::vector<std::string> & reads, std::string strF = FOLDER_NAME);//读如数据

  //取得部分字符串
  static std::string FindFirstStr(std::string & strSrc,int n,bool isBlank = false);//isBlank 是否保留空格
  static std::string FindLastStr(std::string & strSrc,int n,bool isBlank = false);

  /*
   获取目录里所有文件名，
   dir：目录
   filenames：文件列表
   ：是否遍历子目录
   isImage:是否只返回图片文件
  */

  static int Get_filenames(const std::string& dir, std::vector<std::string>& filenames,bool isSubDir = true ,bool isImage = false); 

  //删除一个文件
  static bool DeleteFile(const std::string & strPath);

  static bool FileIsExist(const std::string & strPath);//判断文件是否存在

  static void CreatFile(const std::string & strPath);

};

//内存检测 监视

#define MEMORY_CHICK \
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);\
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;\
	_CrtSetDbgFlag(tmpFlag);
#endif

