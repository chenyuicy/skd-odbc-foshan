
// SKD-sql-GenerateDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SKD-sql-Generate.h"
#include "SKD-sql-GenerateDlg.h"
#include "afxdialogex.h"
#include "../../driver/escaping/escape_sequences.h"
using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSKDsqlGenerateDlg 对话框



CSKDsqlGenerateDlg::CSKDsqlGenerateDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SKDSQLGENERATE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);
}

void CSKDsqlGenerateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSKDsqlGenerateDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//ON_BN_CLICKED(IDC_BUTTON1, &CSKDsqlGenerateDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CSKDsqlGenerateDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_Clear, &CSKDsqlGenerateDlg::OnBnClickedClear)
END_MESSAGE_MAP()


// CSKDsqlGenerateDlg 消息处理程序

BOOL CSKDsqlGenerateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSKDsqlGenerateDlg::OnPaint()
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
HCURSOR CSKDsqlGenerateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}





void CSKDsqlGenerateDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CEdit* pBoxIn;
	pBoxIn = (CEdit*)GetDlgItem(IDC_EDIT1);

	//取值
	CString strIn;
	pBoxIn->GetWindowText(strIn);

	CEdit* pBoxOut;
	pBoxOut = (CEdit*)GetDlgItem(IDC_EDIT2);
	string str = CT2A(strIn.GetBuffer());
	str = replaceEscapeSequences(str);
	CString strOut;
	strOut = str.c_str();
	//赋值
	pBoxOut-> SetWindowText(strOut);


}


void CSKDsqlGenerateDlg::OnBnClickedClear()
{
	// TODO: 在此添加控件通知处理程序代码
	//GetDlgItem(IDC_EDIT1)->SetWindowText("");
	SetDlgItemText(IDC_EDIT1, _T(""));
	SetDlgItemText(IDC_EDIT2, _T(""));
}
