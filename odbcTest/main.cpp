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
功能说明：
1.数据库操作中的添加，修改，删除，主要体现在SQL语句上
2.采用直接执行方式和参数预编译执行方式两种
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

	UCHAR   szDSN[30] = "skm"; //数据源名称  
	UCHAR userID[10] = "default";//数据库用户ID  
	UCHAR passWORD[10] = "";//用户密码  

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
	//2.连接句柄      
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);
	retcode = SQLConnect(hdbc1, szDSN, SQL_NTS, userID, SQL_NTS, passWORD, SQL_NTS);
	//判断连接是否成功     
	//if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	//{
	//	printf("连接失败!\n");
	//}
	//else
	{
		/*
		1.分配一个语句句柄(statement handle)
		2.创建SQL语句
		3.执行语句
		4.销毁语句
		*/
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);
		//char sql[100] = "insert into test values(22,22)";
		char sql[200] = "select {fn CONVERT(((UB)), SQL_BIGINT)} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
		//char sql[200] = "select {fn FLOOR({fn CAST(UB AS DateTime)})} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
		//char sql[200] = "select {fn CONVERT(CASE WHEN ( UB< 210) THEN 1 ELSE 0 END, SQL_BIGINT)} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";
		//char sql[200] = "select {fn FLOOR(UB,10)} AS V from(select UB  from F_PI_PW_HL_6Y_L limit 5)";

		/*
		这里需要在数据库中有test表，要事先建好哦。
		楼主当时就是卡在这里，因为我的默认数据是master，但是我一直操作的是test数据库中的test表，所以一直失败。
		大家一定要注意，如果创建数据源的时候是默认的master数据库，而要操作test数据库中的表，
		要”use test insert into test//values(2,1)“
		*/


		retcode = SQLExecDirect(hstmt1, (SQLCHAR *)sql, strlen(sql));
		printf("操作成功!");
		//释放语句句柄   
		retcode = SQLCloseCursor(hstmt1);
		retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
	}
	//3.断开数据库连接  
	/*
	1. 断开数据库连接
	2.释放连接句柄.
	3.释放环境句柄(如果不再需要在这个环境中作更多连接)
	*/
	SQLDisconnect(hdbc1);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
	return(0);
}