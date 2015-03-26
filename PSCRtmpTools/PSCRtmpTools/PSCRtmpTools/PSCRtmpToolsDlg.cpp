
// PSCRtmpToolsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PSCRtmpTools.h"
#include "PSCRtmpToolsDlg.h"
#include "afxdialogex.h"
#include "LogDlg.h"
#include "..\src\tools\MyDump.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPSCRtmpToolsDlg 对话框



CPSCRtmpToolsDlg::CPSCRtmpToolsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPSCRtmpToolsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPSCRtmpToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_START, m_btnStart);
	DDX_Control(pDX, IDC_PAUSS, m_btnPause);
	DDX_Control(pDX, IDC_STOP, m_btnStop);
}

BEGIN_MESSAGE_MAP(CPSCRtmpToolsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CPSCRtmpToolsDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_START, &CPSCRtmpToolsDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_PAUSS, &CPSCRtmpToolsDlg::OnBnClickedPauss)
	ON_BN_CLICKED(IDC_STOP, &CPSCRtmpToolsDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_REFRESH, &CPSCRtmpToolsDlg::OnBnClickedRefresh)
END_MESSAGE_MAP()


// CPSCRtmpToolsDlg 消息处理程序

BOOL CPSCRtmpToolsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);


	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);//dmp
	//_CrtSetBreakAlloc(1896);//1743 1686 2734 1896 1869

	CLogDlg dlg;
	dlg.m_libRtmp = m_MainEcoder.GetLiveEcode();
	if (dlg.DoModal()== IDCANCEL)
		OnCancel();

	m_MainEcoder.Init();


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPSCRtmpToolsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPSCRtmpToolsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPSCRtmpToolsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPSCRtmpToolsDlg::OnBnClickedCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	if (MessageBox("确认停止？", "提示", MB_ICONEXCLAMATION | MB_OKCANCEL) == IDOK)
	{
		m_MainEcoder.Stop();
		CDialogEx::OnCancel();
	}
}


void CPSCRtmpToolsDlg::OnBnClickedStart()
{
	// TODO:  在此添加控件通知处理程序代码
	m_MainEcoder.Start();//开始
	m_btnStart.EnableWindow(FALSE);
	m_btnPause.EnableWindow(TRUE);
}


void CPSCRtmpToolsDlg::OnBnClickedPauss()
{
	// TODO:  在此添加控件通知处理程序代码
	m_MainEcoder.Pause();
	m_btnStart.EnableWindow(TRUE);
	m_btnPause.EnableWindow(FALSE);
}


void CPSCRtmpToolsDlg::OnBnClickedStop()
{
	// TODO:  在此添加控件通知处理程序代码
	if (MessageBox("确认停止？", "提示", MB_ICONEXCLAMATION | MB_OKCANCEL) == IDOK)
	{
		m_MainEcoder.Stop();
		OnCancel();
	}
}


void CPSCRtmpToolsDlg::OnBnClickedRefresh()
{
	// TODO:  在此添加控件通知处理程序代码
	m_MainEcoder.Refresh();
}
