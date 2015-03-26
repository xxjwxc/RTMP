// LogDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PSCRtmpTools.h"
#include "LogDlg.h"
#include "afxdialogex.h"
#include <string>
#include <boost\algorithm\string.hpp>
#include "..\src\tools\MyTools.h"
#include "..\src\Asio\MyClient.h"


// CLogDlg 对话框

IMPLEMENT_DYNAMIC(CLogDlg, CDialog)

CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
{

}

CLogDlg::~CLogDlg()
{
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CLogDlg::OnBnClickedOk)
END_MESSAGE_MAP()


void CLogDlg::Init()
{
	((CEdit *)GetDlgItem(IDC_EDIT1))->SetWindowText(CString(m_myIni.GetStringConfig("serverAdd").c_str()));
}


BOOL CLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	Init();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

// CLogDlg 消息处理程序


void CLogDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	CString cstr;
	((CEdit *)GetDlgItem(IDC_EDIT1))->GetWindowText(cstr);
	std::string str(cstr.GetBuffer());
	cstr.ReleaseBuffer();

	if (str.length() < 1)
	{
		MessageBox("请输入一个rtmp 服务器地址", "提示");
		return;
	}

	boost::replace_all(str,"\\","/");

	if (!boost::istarts_with(str, "rtmp://"))
		str = "rtmp://" + str;

	unsigned int loc = str.find("//");
	if (loc != std::string::npos)
	{
		unsigned int loc1 = str.find("/", loc + 2);

		if (loc1 != std::string::npos)
			CMyClient::s_ip = MyTools::FindLastStr(MyTools::FindFirstStr(str, loc1,true),loc1-loc-2,true);
	}

	if (!m_libRtmp->Open(str.c_str()))//连接失败
	{
		MessageBox("rtmp 链接失败，\n请打开日志信息并 查看 librtmp.log，了解错误原因", "提示");
		return;
	}
	else
	{
		m_myIni.SetStringConfig("serverAdd", str);
		std::shared_ptr<CMyClient> client = CMyClient::instance();
		if (client) client->RunAction();

		CDialog::OnOK();
	}
	//m_libRtmp->Close();

}


