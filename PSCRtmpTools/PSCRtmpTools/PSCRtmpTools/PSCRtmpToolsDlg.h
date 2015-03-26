
// PSCRtmpToolsDlg.h : 头文件
//

#pragma once
#include "..\src\Encoder\MyMainEcoder.h"
#include "afxwin.h"


// CPSCRtmpToolsDlg 对话框
class CPSCRtmpToolsDlg : public CDialogEx
{
// 构造
public:
	CPSCRtmpToolsDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_PSCRTMPTOOLS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
public:
	CMyMainEcode m_MainEcoder;//操作接口主类
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedPauss();
	afx_msg void OnBnClickedStop();
	CButton m_btnStart;
	CButton m_btnPause;
	CButton m_btnStop;
	afx_msg void OnBnClickedRefresh();
};
