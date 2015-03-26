
#ifndef __MYDUMP_H_
#define __MYDUMP_H_
#include  <dbghelp.h> 
#include <winnt.h>
//#include <boost/format.hpp>

#pragma comment(lib,  "dbghelp.lib")


//设置异常处理回调函数
//SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
//如果无效 			
//调用  在设置自己的异常处理函数后，调用DisableSetUnhandledExceptionFilter 禁止CRT 设置即可。

//异常处理代码
//EXCEPTION_EXECUTE_HANDLER equ 1 表示我已经处理了异常,可以优雅地结束了 
//EXCEPTION_CONTINUE_SEARCH equ 0 表示我不处理,其他人来吧,于是windows调用默认的处理程序显示一个错误框,并结束 
//EXCEPTION_CONTINUE_EXECUTION equ -1 表示错误已经被修复,请从异常发生处继续执行 
long   __stdcall   MyUnhandledExceptionFilter(_EXCEPTION_POINTERS*   ExceptionInfo)
{
	MessageBoxA(NULL, "程序出现了异常，请将exe同目录下error.dmp文件发给开谢小军！谢谢", "提示", MB_OK);

	std::string str = "error.dmp";

	HANDLE   hFile = CreateFile(str.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION   ExInfo;

		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = ExceptionInfo;
		ExInfo.ClientPointers = NULL;

		//   write   the   dump
		BOOL   bOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
		CloseHandle(hFile);
	}  

	//theApp.PostThreadMessage(WM_QUIT, 0, 0);
	return EXCEPTION_EXECUTE_HANDLER;
}

void DisableSetUnhandledExceptionFilter()
{
	void *addr = (void*)GetProcAddress(LoadLibrary(_T("kernel32.dll")),
		"SetUnhandledExceptionFilter");
	if (addr)
	{
		unsigned char code[16];
		int size = 0;
		code[size++] = 0x33;
		code[size++] = 0xC0;
		code[size++] = 0xC2;
		code[size++] = 0x04;
		code[size++] = 0x00;

		DWORD dwOldFlag, dwTempFlag;
		VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
		WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
		VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
	}
}

#endif
