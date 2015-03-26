#include "stdAfx.h"
#include "MyTools.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

std::string GetModulePath( std::string folder,std::string fname)
{
	//std::string path = boost::filesystem::initial_path<boost::filesystem::path>().string();
// 	HMODULE module = GetModuleHandle(0); 
// 	CHAR buf[MAX_PATH]; 
// 	GetModuleFileName(module, buf, sizeof buf); 
	CString strPath;
	::GetModuleFileName( NULL, strPath.GetBuffer(MAX_PATH), MAX_PATH );
	strPath.ReleaseBuffer();
	strPath = strPath.Left(strPath.ReverseFind('\\'));
	std::string path = strPath.GetBuffer();
	strPath.ReleaseBuffer();
	path.append("\\");
	path+=folder;
	if(!boost::filesystem::exists(path))
	{
		boost::filesystem::create_directory(path);
	}

	if (folder!="")
		path+="/";

	path+=fname;//初始化path

	return path;
}

void MyTools::PrintToFile( std::string fname,std::vector<std::string> & prints,bool isClear, std::string strF)
{
	setlocale(LC_ALL, "chs");
	std::string path=GetModulePath(strF,fname);
	//path = "\\\\QGFPVZ3ZLRZTTEH\\iPad\\TXT\\"+ fname;
	namespace newfs = boost::filesystem;
	newfs::ofstream ofs(path.c_str(),(isClear?(std::ios_base::out|std::ios::trunc):(std::ios_base::app|std::ios::ate)));
	if (ofs.is_open())//如果打开了
	{
		std::vector<std::string>::const_iterator it = prints.begin();
		while(it != prints.end())
		{
			ofs<<*it<<std::endl;
			it++;
		}
		ofs.close();
	}
	else//没有打开
		assert(false);
}

bool MyTools::ReadFromFile( std::string fname,std::vector<std::string> & reads, std::string strF)
{
	reads.clear();//清除老元素

	setlocale(LC_ALL, "chs");
	std::string path=GetModulePath(strF,fname);
	namespace newfs = boost::filesystem;
	newfs::ifstream ifs(path.c_str(),std::ios::in);
	std::string str;
	if (ifs.is_open())//如果打开了
	{
		while(!ifs.eof())
		{
			std::getline(ifs,str);
			if (str.length() >0)//空行取消存取
				reads.push_back(str);
		}
		ifs.close();

		return true;
	}
	else//没有打开
		return false;

}

std::string MyTools::FindFirstStr( std::string & strSrc,int n,bool isBlank )
{
	boost::iterator_range<std::string::iterator> ir;
	ir = boost::find_head(strSrc,n);
	std::string str = std::string(ir.begin(),ir.end());
	if (!isBlank)
	{
		boost::trim_left(str);
		boost::trim_right(str);
	}
	return str;
}

std::string MyTools::FindLastStr( std::string & strSrc,int n,bool isBlank )
{
	boost::iterator_range<std::string::iterator> ir;
	ir = boost::find_tail(strSrc,n);
	std::string str = std::string(ir.begin(),ir.end());
	if (!isBlank)
	{
		boost::trim_left(str);
		boost::trim_right(str);
	}
	return str;
}

int MyTools::Get_filenames( const std::string& dir, std::vector<std::string>& filenames,bool isSubDir /*= true */,bool isImage /*= false*/ )
{
	filenames.clear();
	boost::filesystem::path path(dir);  
	if (!boost::filesystem::exists(path))  
	{ 
		boost::filesystem::create_directories(path);//创建目录	
		//boost::filesystem::create_directory(path);//创建目录(不能创建数型目录)	
		return -1;
	}  

	boost::filesystem::directory_iterator end_iter;  
	for (boost::filesystem::directory_iterator iter(path); iter!=end_iter; ++iter)  
	{  
		if (boost::filesystem::is_regular_file(iter->status()))  
		{
			if (!isImage || boost::icontains(".JPEG.JPG.TIFF.TIF.BMP.GIF.PNG",iter->path().extension().string()))
				filenames.push_back(iter->path().filename().string());  
		}  
		else if (boost::filesystem::is_directory(iter->status()))  
		{ 
			if (isSubDir)//查找子目录
			{
				Get_filenames(iter->path().string(), filenames, isSubDir,isImage); 
			}
			else
			{
				//boost::filesystem::remove(iter->path());//删除文件
				boost::filesystem::remove_all(iter->path());//删除目录
			} 
		}  
	}  

	return filenames.size();  
}

bool MyTools::DeleteFile( const std::string & strPath )
{
	boost::filesystem::path path(strPath);
	if (boost::filesystem::exists(path))  
		boost::filesystem::remove(path);//删除文件

	return true;
}

bool MyTools::FileIsExist( const std::string & strPath )
{
	boost::filesystem::path path(strPath);
	return boost::filesystem::exists(path);
}

/*
  获取自己模块句柄
*/
HMODULE GetSelfModuleHandle()
{
	MEMORY_BASIC_INFORMATION mbi;
	return ((::VirtualQuery(GetSelfModuleHandle, &mbi, sizeof(mbi)) != 0) ? (HMODULE) mbi.AllocationBase : NULL);
}

CString MyTools::GetSelfModulePath()
{
	char ctr[MAX_PATH+1] = {0};
	HMODULE hModule = GetSelfModuleHandle();
	GetModuleFileName(hModule,ctr,MAX_PATH);
	CString str;
	str.Format("%s",ctr);
	str = str.Left(str.ReverseFind('\\'));
	return str;
}

void MyTools::CreatFile( const std::string & strPath )
{
	if (!FileIsExist(strPath))
		boost::filesystem::create_directory(strPath);

}

