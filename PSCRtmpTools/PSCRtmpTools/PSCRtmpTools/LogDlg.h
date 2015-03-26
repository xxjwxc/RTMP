#pragma once
#include <memory>
#include "..\src\librtmp\LibRtmp.h"
#include "..\src\tools\MyIni.h"


// CLogDlg 对话框

class CLogDlg : public CDialog
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLogDlg();

// 对话框数据
	enum { IDD = IDD_LOG_DIALOG };

	std::shared_ptr<LibRtmp> m_libRtmp = nullptr;
	CMyIni m_myIni;
	void Init();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
