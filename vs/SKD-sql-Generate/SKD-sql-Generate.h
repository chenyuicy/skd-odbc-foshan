
// SKD-sql-Generate.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSKDsqlGenerateApp: 
// �йش����ʵ�֣������ SKD-sql-Generate.cpp
//

class CSKDsqlGenerateApp : public CWinApp
{
public:
	CSKDsqlGenerateApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSKDsqlGenerateApp theApp;