/********************************************************************
	创建时间：	2014/08/05  17:40:22 
	文件名： 	HostMsPaintChild.h
	作者：		谢小军

	功能：		1. 用于嵌套一个子窗口到父窗口
	2.

	说明：	    1.
	2.
*********************************************************************/

#ifndef HOSTMSPAINTCHILD_H_
#define HOSTMSPAINTCHILD_H_

/*************Global functions for hosting******************/
class ChostChildDlg
{	
public:
	//创建父窗口
	//args 必须以空格开头
	bool ActionHostmspaint(HWND parent,const CRect & rect,LPCTSTR path,LPCTSTR name,LPCTSTR args = "");

	//销毁
	void ActionKillprocess();
public:
	ChostChildDlg();
	~ChostChildDlg();

private:
	HANDLE m_handle;//process handle
	static HWND s_apphwnd;//window handle
	//Function to enumerate all windows.
	static int CALLBACK EnumWindowsProc(HWND hwnd, LPARAM param)
	{
		DWORD pID;
		DWORD TpID = GetWindowThreadProcessId(hwnd, &pID);//get process id
		if (TpID == (DWORD)param)
		{
			s_apphwnd=hwnd;//hwnd is the window handle
			return false;
		}
		return true;
	}
	
	//Functio to start a orocess and return the process handle
	HANDLE StartProcess(LPCTSTR program,LPCTSTR filename, LPCTSTR args);
};

/**********************************************************/

#endif