#include <stdio.h>      
#include <string.h>      
#include <windows.h>      
#include <sql.h>     
#include <sqlext.h>      
#include <sqltypes.h>      
#include <odbcss.h>  
#include <iostream>
#include <regex>
#define MAXBUFLEN 255  

typedef RETCODE(*FuncSQLAllocHandle)(SQLSMALLINT handle_type, SQLHANDLE input_handle, SQLHANDLE * output_handle);
typedef RETCODE(*FuncSQLExecDirect)(HSTMT statement_handle, SQLTCHAR * statement_text, SQLINTEGER statement_text_size);
typedef RETCODE(*FuncSQLSetEnvAttr)(SQLHENV handle, SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER value_length);
typedef RETCODE(*FuncSQLConnect)(HDBC connection_handle,
	SQLTCHAR * dsn,
	SQLSMALLINT dsn_size,
	SQLTCHAR * user,
	SQLSMALLINT user_size,
	SQLTCHAR * password,
	SQLSMALLINT password_size);

/*
����˵����
1.���ݿ�����е���ӣ��޸ģ�ɾ������Ҫ������SQL�����
2.����ֱ��ִ�з�ʽ�Ͳ���Ԥ����ִ�з�ʽ����
*/
using namespace std;

int main()
{
	HMODULE hModule = LoadLibrary("skm_odbc.dll");
	if (!hModule)
	{
		cout << "Error!" << endl;
	}
	FuncSQLAllocHandle SQLAllocHdl = (FuncSQLAllocHandle)GetProcAddress(hModule, "SQLAllocHandle");
	FuncSQLExecDirect SQLExec = (FuncSQLExecDirect)GetProcAddress(hModule, "SQLExecDirect");
	FuncSQLSetEnvAttr SQLSetEnv = (FuncSQLSetEnvAttr)GetProcAddress(hModule, "SQLSetEnvAttr");
	FuncSQLConnect SQLCon = (FuncSQLConnect)GetProcAddress(hModule, "SQLConnect");

	//	string result = replaceEscapeSequences(str);

	SQLHENV henv = SQL_NULL_HENV;
	SQLHDBC hdbc1 = SQL_NULL_HDBC;
	SQLHSTMT hstmt1 = SQL_NULL_HSTMT;
	RETCODE retcode;

	UCHAR   szDSN[30] = "skm"; //����Դ����  
	UCHAR userID[10] = "default";//���ݿ��û�ID  
	UCHAR passWORD[10] = "";//�û�����  

	retcode = SQLAllocHdl(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	retcode = SQLSetEnv(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);    
	retcode = SQLAllocHdl(SQL_HANDLE_DBC, henv, &hdbc1);
	retcode = SQLCon(hdbc1, szDSN, SQL_NTS, userID, SQL_NTS, passWORD, SQL_NTS);
	retcode = SQLAllocHdl(SQL_HANDLE_STMT, hdbc1, &hstmt1);
	//char sql[200] = "select * from default.F_PI_PW_HL_6Y_R  where (DATADATE BETWEEN({ts '2016-0009-04 11:25:12' }) AND({d '2016-0011-21' }))";

	char sql[200] = "select {fn CONVERT((({fn FLOOR({fn ROUND({fn SQRT(UB)})})})), SQL_BIGINT)} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
	//char sql[200] = "select {fn FLOOR({fn ROUND({fn SQRT(UB)})})} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
	//char sql[200] = "select {fn CONVERT(CASE WHEN ( UB< 210) THEN 1 ELSE 0 END, SQL_BIGINT)} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
	//char sql[200] = "select {fn FLOOR((UB),10)} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
	
	retcode = SQLExec(hstmt1, (SQLCHAR *)sql, strlen(sql));





	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	//2.���Ӿ��      
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);
	retcode = SQLConnect(hdbc1, szDSN, SQL_NTS, userID, SQL_NTS, passWORD, SQL_NTS);
	//�ж������Ƿ�ɹ�     
	//if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	//{
	//	printf("����ʧ��!\n");
	//}
	//else
	{
		/*
		1.����һ�������(statement handle)
		2.����SQL���
		3.ִ�����
		4.�������
		*/
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);
		//char sql[100] = "insert into test values(22,22)";
		char sql[200] = "select {fn CONVERT(((UB)), SQL_BIGINT)} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
		//char sql[200] = "select {fn FLOOR({fn CAST(UB AS DateTime)})} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
		//char sql[200] = "select {fn CONVERT(CASE WHEN ( UB< 210) THEN 1 ELSE 0 END, SQL_BIGINT)} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
		//char sql[200] = "select {fn FLOOR(UB,10)} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";

		/*
		������Ҫ�����ݿ�����test��Ҫ���Ƚ���Ŷ��
		¥����ʱ���ǿ��������Ϊ�ҵ�Ĭ��������master��������һֱ��������test���ݿ��е�test������һֱʧ�ܡ�
		���һ��Ҫע�⣬�����������Դ��ʱ����Ĭ�ϵ�master���ݿ⣬��Ҫ����test���ݿ��еı�
		Ҫ��use test insert into test//values(2,1)��
		*/


		retcode = SQLExecDirect(hstmt1, (SQLCHAR *)sql, strlen(sql));
		printf("�����ɹ�!");
		//�ͷ������   
		retcode = SQLCloseCursor(hstmt1);
		retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
	}
	//3.�Ͽ����ݿ�����  
	/*
	1. �Ͽ����ݿ�����
	2.�ͷ����Ӿ��.
	3.�ͷŻ������(���������Ҫ���������������������)
	*/
	SQLDisconnect(hdbc1);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
	return(0);
}