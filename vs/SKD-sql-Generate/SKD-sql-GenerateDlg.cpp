
// SKD-sql-GenerateDlg.cpp : ʵ���ļ�
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


// CSKDsqlGenerateDlg �Ի���



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


// CSKDsqlGenerateDlg ��Ϣ�������

BOOL CSKDsqlGenerateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSKDsqlGenerateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSKDsqlGenerateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}





void CSKDsqlGenerateDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CEdit* pBoxIn;
	pBoxIn = (CEdit*)GetDlgItem(IDC_EDIT1);

	//ȡֵ
	CString strIn;
	pBoxIn->GetWindowText(strIn);

	CEdit* pBoxOut;
	pBoxOut = (CEdit*)GetDlgItem(IDC_EDIT2);
	string str = CT2A(strIn.GetBuffer());
	str = replaceEscapeSequences(str);
	CString strOut;
	strOut = str.c_str();
	//��ֵ
	pBoxOut-> SetWindowText(strOut);


}


void CSKDsqlGenerateDlg::OnBnClickedClear()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//GetDlgItem(IDC_EDIT1)->SetWindowText("");
	SetDlgItemText(IDC_EDIT1, _T(""));
	SetDlgItemText(IDC_EDIT2, _T(""));
}
