#include "StdAfx.h"
#include "HostMsPaintChild.h"

HWND ChostChildDlg::s_apphwnd = NULL;//window handle

ChostChildDlg::ChostChildDlg()
{
	m_handle = NULL;
}

ChostChildDlg::~ChostChildDlg()
{
	ActionKillprocess();
}

//Functio to start a orocess and return the process handle
HANDLE ChostChildDlg::StartProcess(LPCTSTR program,LPCTSTR filename, LPCTSTR args)
{
	CString str = "";
	str.Format("%s%s",program,filename);
	HANDLE hProcess = NULL;
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startupInfo;
	::ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	if(::CreateProcess(str, (LPTSTR)args, 
		NULL,  // process security
		NULL,  // thread security
		FALSE, // no inheritance
		0,     // no startup flags
		NULL,  // no special environment
		program,  // default startup directory
		&startupInfo,
		&processInfo))
	{ /* success */
		for (int i = 0; i < 15000; i++)
		{
			Sleep(100);
			//::WaitForInputIdle(processInfo.hProcess, INFINITE);
			::EnumWindows(&EnumWindowsProc, processInfo.dwThreadId);//Iterate all windows
			hProcess = processInfo.hProcess;
			if (s_apphwnd)		//找到相应的窗口
			{
				//Sleep(40);
				break;
			}
		}
	} /* success */
	return hProcess;
}

bool ChostChildDlg::ActionHostmspaint(HWND parent,const CRect & rect,LPCTSTR path,LPCTSTR name,LPCTSTR args)
{
	m_handle=StartProcess(path,name,args);//Start ms paint
	if(s_apphwnd!=NULL)//check for window handle
	{
		::SetParent(s_apphwnd,parent);//set parent of ms paint to our dialog.
		::WaitForInputIdle(m_handle, INFINITE);
		SetWindowLong(s_apphwnd, GWL_STYLE, WS_VISIBLE);//eraze title of ms paint window.
		//Positioning ms paint.
		::MoveWindow(s_apphwnd, rect.left, rect.top,rect.right, rect.bottom, true);
		return true;
	}

	return false;
}

void ChostChildDlg::ActionKillprocess()
{
	TerminateProcess(m_handle,0);	//kill the process using handle
	s_apphwnd = NULL;
}
