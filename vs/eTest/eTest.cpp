// eTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../driver/escaping/escape_sequences.h"
#include "../../driver/log.h"
#include <windows.h>
#include <stringapiset.h>

#include <iostream>
#include <map>
#include <list>
using namespace std;


int main()
{

	//string S = "2012-0001-0012 11:0010:0050";
	//date d;
	string sql = "SELECT `白坭所配变运行量测数`.`NAME` AS `X_________`,  (CASE WHEN COUNT(`白坭所配变运行量测数`.`DATATIME`) = 0 THEN NULL ELSE CAST(SUM({fn CONVERT((CASE WHEN(`白坭所配变运行量测数`.`U_INBAL_FACTOR` > 2) THEN 1 ELSE 0 END), SQL_BIGINT) }) AS FLOAT) / COUNT(`白坭所配变运行量测数`.`DATATIME`) END) AS `usr_____________________ok` FROM `Y2D3OUT__F_PI_PW_HL` `白坭所配变运行量测数` WHERE((`白坭所配变运行量测数`.`_UA` >= 165) AND(`白坭所配变运行量测数`.`_UA` <= 300) AND(`白坭所配变运行量测数`.`_UB` >= 165) AND(`白坭所配变运行量测数`.`_UB` <= 300) AND(`白坭所配变运行量测数`.`_UC` >= 165) AND(`白坭所配变运行量测数`.`_UC` <= 300) AND(`白坭所配变运行量测数`.`ZONE_ID` = '06110') AND(`白坭所配变运行量测数`.`DATATIME` >= {ts '2017-12-29 00:00:00'}) AND(`白坭所配变运行量测数`.`DATATIME` <= {ts '2017-12-31 00:00:00'})) GROUP BY `X_________`";

	//std::string sql = "SELECT (CASE WHEN ((`白坭所配变运行量测数`.`_UA` > 245) OR (`白坭所配变运行量测数`.`_UA` < 210) OR (`白坭所配变运行量测数`.`_UB` > 245) OR (`白坭所配变运行量测数`.`_UB` < 210) OR (`白坭所配变运行量测数`.`_UC` > 245) OR (`白坭所配变运行量测数`.`_UC` < 210)) THEN 1 ELSE 0 END) AS `Calculation_2260626120000000`,\
	//	COUNT(DISTINCT `白坭所配变运行量测数`.`NAME`) AS `ctd____________ok`,\
	//{fn TIMESTAMPADD(SQL_TSI_DAY, CAST({fn TRUNCATE((-1 * (EXTRACT(DAY FROM `白坭所配变运行量测数`.`DATADATE`) - 1)),0) } AS INTEGER), CAST(`白坭所配变运行量测数`.`DATADATE` AS TIMESTAMP))} AS `tmn____ok`\
	//	FROM `D3VW2_SUB__F_PI_PW_HL` `白坭所配变运行量测数`\
	//	WHERE((`白坭所配变运行量测数`.`_UA` >= 165) AND(`白坭所配变运行量测数`.`_UA` <= 300) AND(`白坭所配变运行量测数`.`_UB` >= 165) AND(`白坭所配变运行量测数`.`_UB` <= 300) AND(`白坭所配变运行量测数`.`_UC` >= 165) AND(`白坭所配变运行量测数`.`_UC` <= 300))\
	//	GROUP BY `Calculation_2260626120000000`,\
	//	`tmn____ok`";
	//std::string sql = "CAST({fn TRUNCATE((-1 * (EXTRACT(DAY FROM `白坭所配变运行量测数`.`DATADATE`) - 1)),0) } AS TIMESTAMP)";
	//string sql = "CAST({fn TRUNCATE(EXTRACT(HOUR FROM CAST(`自定义 SQL 查询`.`时间` AS TIMESTAMP)),0)} AS INTEGER) AS `hr____ok`";
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, sql.c_str(), sql.length(), NULL, 0);
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	::MultiByteToWideChar(CP_UTF8, NULL, sql.c_str(), sql.length(), wszString, wcsLen);
	wszString[wcsLen] = '\0';
	
	//sql = "{d '002017-0012-0029'}";


	string ret = replaceEscapeSequences(sql);

    return 0;
}

