/*

https://docs.faircom.com/doc/sqlref/#33384.htm
https://docs.microsoft.com/ru-ru/sql/odbc/reference/appendixes/time-date-and-interval-functions
https://my.vertica.com/docs/7.2.x/HTML/index.htm#Authoring/SQLReferenceManual/Functions/Date-Time/TIMESTAMPADD.htm%3FTocPath%3DSQL%2520Reference%2520Manual%7CSQL%2520Functions%7CDate%252FTime%2520Functions%7C_____43

*/
#include "escape_sequences.h"

#include <iostream>
#include <map>
#include "lexer.h"
#include "../log.h"
#include <list>
#include <windows.h>
#include <stringapiset.h>
#include <boost/regex.hpp>

#include <iostream>


namespace {

	const std::map<const Token::Type, const std::string> fn_curly_map{
		{ Token::FN, "" },
		{ Token::TS, "toDateTime" },
		{ Token::D , "toDate" }
	};

	//函数映射表
	const std::map<const Token::Type, const std::string> function_map{
		{ Token::CONVERT,"convert" },
		{ Token::LTRIM,"" },
		{ Token::EXTRACT,"extract" },
		{ Token::CAST,"cast" },
		{ Token::DECODE,"transform" },
		{ Token::ROUND, "round" },
		{ Token::FLOOR, "floor" },
		{ Token::POWER, "pow" },
		{ Token::TRUNCATE, "trunc" },
		{ Token::SQRT, "sqrt" },
		{ Token::ABS, "abs" },
		{ Token::CONCAT, "concat" },
		{ Token::COALESCE ,"ifNull" },
		{ Token::SPLIT,"splitByString" },
		{ Token::LEFT,"substringUTF8" },
		{ Token::CURDATE, "today" },
		{ Token::TIMESTAMPDIFF, "dateDiff" },
		{ Token::TIMESTAMPADD, "dateAdd" },
		{ Token::DAYOFYEAR,"dayofyear" },
		{ Token::SIGN,"multiIf" },

	};

	//函数对应的分隔符 未列出为Token::COMMA
	const std::map<const Token::Type, const Token::Type> function_map_splitter{
		{ Token::EXTRACT,  Token::FROM },
		{ Token::CAST,  Token::AS },
	};

	//常量映射
	const std::map<const Token::Type, const std::string> function_map_strip_params{
		{ Token::CURRENT_TIMESTAMP, "toUnixTimestamp(now())" },
	};

	//convert函数的参数表
	const std::map<const std::string, const std::string> fn_convert_map{
		{ "SQL_TINYINT", "toUInt8" },
		{ "SQL_SMALLINT", "toUInt16" },
		{ "SQL_INTEGER", "toInt32" },
		{ "SQL_BIGINT", "toInt64" },
		{ "SQL_REAL", "toFloat32" },
		{ "SQL_DOUBLE", "toFloat64" },
		{ "SQL_VARCHAR", "toString" },
		{ "SQL_DATE", "toDate" },
		{ "SQL_TYPE_DATE", "toDate" },
		{ "SQL_TIMESTAMP", "toDateTime" },
		{ "SQL_TYPE_TIMESTAMP", "toDateTime" },
	};

	//extract函数的参数表
	const std::map<const std::string, const std::string> fn_extract_map{
		{ "YEAR",  "toYear" },
		{ "MONTH",  "toMonth" },
		{ "DAY",  "toDayOfMonth" },
		{ "HOUR","toHour" },
		{ "MINUTE","toMinute" },
		{ "SECOND","toSecond" },
		{ "'year'",  "toYear" },
		{ "'month'",  "toMonth" },
		{ "'day'",  "toDayOfMonth" },
		{ "'hour'",  "toHour" },
		{ "'minute'",  "toMinute" },
		{ "'second'",  "toSecond" }
	};

	//timeadd函数的参数表
	const std::map<const Token::Type, const std::string> fn_timeadd_map{
		// {Token::SQL_TSI_FRAC_SECOND, ""},
		{ Token::SQL_TSI_SECOND,  "addSeconds" },
		{ Token::SQL_TSI_MINUTE,  "addMinutes" },
		{ Token::SQL_TSI_HOUR,    "addHours" },
		{ Token::SQL_TSI_DAY,     "addDays" },
		{ Token::SQL_TSI_WEEK,    "addWeeks" },
		{ Token::SQL_TSI_MONTH,   "addMonths" },
		{ Token::SQL_TSI_QUARTER, "addQuarters" },
		{ Token::SQL_TSI_YEAR,    "addYears" },
	};

	string removeMilliseconds(const StringView token);

	//string processFunc(const StringView seq, Lexer & lex);
	//string processParam(const StringView seq, Lexer & lex);

	//替换where之后group之前的 DATATIME->DATADATE TS->d
	string ReplaceDataTime(string str)
	{
		string s = to_upper(str);
		int posWhere = s.find("WHERE");
		if (posWhere==-1)
		{
			return str;
		}

		int posGroup = s.find("GROUP");
		if (posGroup==-1)
		{
			posGroup = s.length() - 1;
		}

		string bef1 = "`DATATIME` >= {ts ";
		string bef2 = "`DATATIME` <= {ts ";

		string aft1 = "`DATADATE` >= {d ";
		string aft2 = "`DATADATE` <= {d ";

		int posReplace = posWhere;
		while ((str.find(bef1, posReplace) < posGroup)
			&& (str.find(bef1, posReplace) != -1))
		{
			str = str.replace(str.find(bef1, posReplace), bef1.length(), aft1);
			posReplace += aft1.length();
		}

		posReplace = posWhere;
		while ((str.find(bef2, posReplace) < posGroup)
			&& (str.find(bef2, posReplace) != -1))
		{
			str = str.replace(str.find(bef2, posReplace), bef2.length(), aft2);
			posReplace += aft2.length();
		}

		return str;
	}

	//时间判断
	bool isDataTime(string str)
	{
		regex expression("(\\d{4})-(0\\d{1}|1[0-2])-(0\\d{1}|[12]\\d{1}|3[01])\\s(0\\d{1}|1\\d{1}|2[0-3]):[0-5]\\d{1}:([0-5]\\d{1})");
		cmatch what;
		if (regex_match(str.c_str(), what, expression))
			return true;
		else
			return false;
	}

	//拆分至年月日时分秒 并修正
	date StringToDate(string str)
	{
		//去掉''
		str.erase(std::remove(str.begin(), str.end(), '\''), str.end());

		date d;
		vector<string> destination;
		split(destination, str, is_any_of(" -:."));
		if (destination.size() >= 3)
		{
			if (destination.at(0).size() > 4)
			{
				d.year = destination.at(0).substr(destination.at(0).size() - 4, 4);
			}
			else
			{
				d.year = destination.at(0);
			}

			if (destination.at(1).size() > 2)
			{
				d.month = destination.at(1).substr(destination.at(1).size() - 2, 2);
			}
			else
			{
				d.month = destination.at(1);
			}

			if (destination.at(2).size() > 2)
			{
				d.day = destination.at(2).substr(destination.at(2).size() - 2, 2);
			}
			else
			{
				d.day = destination.at(2);
			}

			if (destination.size() >= 6)
			{
				if (destination.at(3).size() > 2)
				{
					d.hour = destination.at(3).substr(destination.at(3).size() - 2, 2);
				}
				else
				{
					d.hour = destination.at(3);
				}

				if (destination.at(4).size() > 2)
				{
					d.minute = destination.at(4).substr(destination.at(4).size() - 2, 2);
				}
				else
				{
					d.minute = destination.at(4);
				}

				if (destination.at(5).size() > 2)
				{
					d.second = destination.at(5).substr(destination.at(5).size() - 2, 2);
				}
				else
				{
					d.second = destination.at(5);
				}
			}
			else
			{
				return d;
			}
		}

		return d;
	}

	//年月日拼加
	string DateToString(date d)
	{
		string s = "";
		if (d.year != ""&&d.month != ""&&d.day != "")
		{
			s = d.year + "-" + d.month + "-" + d.day;
		}
		if (d.hour != ""&&d.minute != ""&&d.second != "")
		{
			s += " " + d.hour + ":" + d.minute + ":" + d.second;
		}
		return "\'"+s+ "\'";
	}
	


	//增加Sample ——2018.06.13
	string AddSample(string str, map<string,list<string>> findstr, map<string, string> repStr)
	{
		int pos = 0;
		bool bfind = false;
		map<string, list<string>>::iterator itorM = findstr.begin();
		while (itorM != findstr.end())
		{
			list<string> cmpStr = itorM->second;

			list<string>::iterator itor = cmpStr.begin();
			while (itor != cmpStr.end())
			{
				if (str.find(*itor) == -1)//can't find sub str
				{
					bfind = false;
					break;
				}
				bfind = true;
				++itor;
			}

			if (bfind == false)
			{
				++itorM;
				continue;
			}

			pos = str.find(itorM->first);
			if (pos != -1)
			{
				string replaceStr = repStr[itorM->first];
				str.insert(pos, replaceStr);
				return str;
			}
			else
			{
				return str;
			}
		}

		return str;
	}

	//消除空格
	std::string& trim(std::string &s)
	{
		if (s.empty())
		{
			return s;
		}

		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" ") + 1);
		return s;
	}

	//消除毫秒
	string removeMilliseconds(const StringView token) {
		if (token.empty()) {
			return string();
		}

		const char * begin = token.data();
		const char * p = begin + token.size() - 1;
		const char * dot = nullptr;
		const bool quoted = (*p == '\'');
		if (quoted) {
			--p;
		}
		for (; p > begin; --p) {
			if (isdigit(*p)) {
				continue;
			}
			if (*p == '.') {
				if (dot) {
					return token.to_string();
				}
				dot = p;
			}
			else {
				if (dot) {
					return string(begin, dot) + (quoted ? "'" : "");
				}
				return token.to_string();
			}
		}

		return token.to_string();
	}

	//处理日期--年月日YYYY-MM-DD ICY add
	string processSub_Date(string function_name, deque<string> &params)
	{
		string result = function_name;
		if (params.size() < 1)
		{
			result += "()";
			return result;
		}

		string d = trim(params.front());
		params.pop_front();
		if (d.size() <= 0)
		{
			result += "()";
			return result;
		}

		if (isDataTime(d))
		{
			result += "(";
			result += removeMilliseconds(d);
			result += ")";
			return result;
		}
		else
		{
			date dat = StringToDate(d);
			d = DateToString(dat);
			result += "(";
			result += removeMilliseconds(d);
			result += ")";
			return result;
		}

		/*size_t pos = d.find("-00");
		if (pos != d.npos) {
			result += "(" + d.replace(pos, 3, string("-")) + ")";
			return result;
		}
		else {
			result += "(" + d + ")";
			return result;
		}*/
	}
	
	//处理时间--年月日YYYY-MM-DD HH:MM:SS ICY add
	string processSub_DateTime(string function_name,deque<string> &params)
	{
		string result = function_name;
		if (params.size()<1)
		{
			result += "()";
			return result;
		}

		string d = trim(params.front());
		params.pop_front();
		if (d.size() <= 0)
		{
			result += "()";
			return result;
		}
	
		if (isDataTime(d))
		{
			result += "(";
			result += removeMilliseconds(d);
			result += ")";
			return result;
		}
		else
		{
			date dat = StringToDate(d);
			d = DateToString(dat);
			result += "(";
			result += removeMilliseconds(d);
			result += ")";
			return result;
		}

		/*size_t pos = d.find("-00");
		if (pos != d.npos) {
			result += "(";
			result += removeMilliseconds(d.replace(pos, 3, string("-")));
			result += ")";
			return result;
		}
		else {
			result += "(";
			result += removeMilliseconds(d);
			result += ")";
			return result;
		}*/
	}
	
	string processSub_Convert(string function_name,deque<string> &params)
	{
		string result = "";
		if (params.size() < 2)
		{
			return result;
		}
		string param1 = params.front();
		params.pop_front();
		string param2 = params.front();
		params.pop_front();
		trim(param2);

		if (fn_convert_map.find(param2) != fn_convert_map.end())
		{
			function_name = fn_convert_map.at(param2);
		}
		result += function_name + "(";
		result += param1 + ")";

		return result;
	}
	
	string processSub_Decode(string function_name,deque<string> &params)
	{
		string result = function_name;
		if (params.size() < 4 || (params.size() % 2 != 0))
		{
			result +="("+ params.at(params.size() - 1)+")";
			return result;
		}
		string x = "";
		string q = "";
		string a = "";
		x = trim(params.front());
		x = "ifNull(" + x + ",0)";
		params.pop_front();
		while (params.size() > 1)
		{
			string p1 = trim(params.front());
			q += p1 == "null" ? "0" : p1;
			params.pop_front();
			string p2 = trim(params.front());
			a += p2 == "null" ? "0" : p2;
			params.pop_front();
			if (params.size() > 1)
			{
				q += ",";
				a += ",";
			}
		}

		result +=  "(";
		result += x + ",";
		result += "[" + q + "]" + ",";
		result += "[" + a + "]" + ",";
		result += trim(params.front());
		params.pop_front();
		result += ")";	

		return result;
	}

	string processSub_Cast(string function_name, deque<string> &params)
	{
		string result = "";
		if (params.size() != 2)
		{
			string param = (params.size() == 0) ? "" : params.front();
			return function_name+"("+ param +")";
		}

		string param1 = trim(params.front());
		params.pop_front();
		string param2 = trim(params.front());
		params.pop_front();
		if (param2 == "TIMESTAMP")
		{
			result += "(" + param1 + ")";
			return result;
		}
		else
		{
			result += "CAST(" + param1 + " AS " + param2 + ")";
			return result;
		}
	}

	string processSub_Extract(string function_name, deque<string> &params)
	{
		string result = "";
		if (params.size() != 2)
		{
			string param = (params.size() == 0) ? "" : params.front();
			return function_name + "(" + param + ")";
		}

		string param1 = to_upper(trim(params.front()));
		params.pop_front();
		string param2 = trim(params.front());
		params.pop_front();

		if (fn_extract_map.find(param1) != fn_extract_map.end())
		{
			function_name = fn_extract_map.at(param1);
		}

		result = function_name + "(";
		result += param2;
		result += ")";

		return result;
	}

	string processSub_Timestampadd(string function_name, deque<string> &params)
	{
		string result="";
		if (params.size() != 3)
		{
			return function_name+"()";
		}
		Lexer l(trim(params.front()));
		Token t = l.GoNext();
		if (fn_timeadd_map.find(t.type) == fn_timeadd_map.end())
		{
			return function_name + "()";
		}
		function_name = fn_timeadd_map.at(t.type);
		params.pop_front();
		string timeadd = trim(params.front());
		params.pop_front();
		string time = trim(params.front());
		params.pop_front();
		result += function_name + "(";
		result += time + "," + timeadd;
		result += ")";

		return result;
	}

	string processSub_Split(string function_name, deque<string> &params)
	{
		string result = "";
		if (params.size() > 3)
			return function_name+"()";
		else if (params.size() == 3)
		{
			result += function_name + "(";
			while (params.size() > 1) {
				result += trim(params.front());
				params.pop_front();
				if (params.size() > 1)result += ",";
			}
			result += ")";
			int n = std::stoi(trim(params.front()));
			n = n - 1;
			result += "[" + to_string(n) + "]";
		}
		else if (params.size() == 2)
		{
			result += function_name + "(";
			while (!params.empty()) {
				result += trim(params.front());
				params.pop_front();
				if (!params.empty())result += ",";
			}
			result += ")";
		}

		return result;
	}

	string processSub_Left(string function_name, deque<string> &params)
	{
		string result = function_name;
		if (params.size()<2)
		{
			string param = (params.size() == 0)?"":params.front();
			return function_name + "(" + param + ")";
		}

		if (params.size() == 2)
		{
			string param1 = params.front();
			params.pop_front();
			string param2 = "1";
			string param3 = params.front();
			params.pop_front();	

			result += "(";
			result += param1 + "," + param2 + "," + param3;
			result += ")";
			return result;
		}
	}

	string processSub_Sign(string function_name, deque<string> &params)
	{
		string result = function_name;
		if (params.size()!=1)
		{
			string param = (params.size() == 0) ? "" : params.front();
			return function_name + "(" + param + ")";
		}

		result += "(";
		result += "(" + params[0] + ">0), 1, (" + params[0] + "<0), -1, 0";
		result += ")";
		return result;
	}



	//数据库中无对应函数，需两个函数拼合 后期在数据库中增加对应函数后删除此函数
	string processSub_Dayofyear(string function_name, deque<string> &params)
	{
		string result = "";
		if (params.size() < 1)
		{
			return function_name+"()";
		}
		string dt = trim(params.front());
		params.pop_front();
		result += "(toDate(" + dt + ")";
		result += "-toStartOfYear(toDate(" + dt + "))+1)";

		return result;
	}

	//对函数名进行分类，分送至processSub处理
	string processFuncName(string function_name, int functiontype, deque<string> &params)
	{
		string result = "";
		switch (functiontype)
		{
		case Token::TS:
		{
			result += processSub_DateTime(function_name,params);
		}
		break;
		case Token::D:
		{
			result += processSub_Date(function_name,params);
		}
		break;
		case Token::DECODE:
		{
			result += processSub_Decode(function_name,params);
		}
		break;
		case Token::SIGN:
		{
			result += processSub_Sign(function_name, params);
		}
		break;
		case  Token::CAST:
		{
			result += processSub_Cast(function_name, params);
		}
		break;
		case Token::CONVERT:
		{
			result += processSub_Convert(function_name, params);
		}
		break;
		case Token::TIMESTAMPADD:
		{
			result += processSub_Timestampadd(function_name, params);
		}
		break;;
		case Token::SPLIT:
		{
			result += processSub_Split(function_name, params);
		}
		break;
		case Token::LEFT:
		{
			result += processSub_Left(function_name, params);
		}
		break;
		case Token::DAYOFYEAR:
		{
			result += processSub_Dayofyear(function_name, params);
		}
		break;
		case Token::EXTRACT:
		{
			result += processSub_Extract(function_name, params);
		}
		break;
		default:
		{
			result += function_name + "(";
			while (!params.empty()) {
				result += trim(params.front());
				params.pop_front();
				if (!params.empty())result += ",";
			}
			result += ")";
		}
		break;
		}

		params.clear();
		return result;
	}

	

	/* 核心递归函数
	inFuncion:是否在函数内部第一层，TRUE查找参数 FALSE返回原字串
	splitter_token：参数分隔符默认为Token::COMMA
	needLoop:是否递归处理 FALSE只处理一层
	*/
	string processFunc(const StringView seq, Lexer & lex, bool inFuncion, int splitter_token, bool needLoop) {
		//lex.SetEmitSpaces(true);
		string result = "";
		Token fn = lex.GoNext();
		//string params[10] = { "" };
		std::deque<string> params;
		int i = 0;
		//int level = (exeuteLevel==0)?1:0;
		int level = 0;
		string word;
		//for (fn = lex.GoNext(); fn.type != Token::EOS; fn=lex.GoNext()) 
		if (fn.type == Token::EOS)
		{
			return result;
		}
		do {
			word = fn.literal.to_string();
			const Token	next = lex.ReadNext();

			if (
				(fn.type == Token::LCURLY && fn_curly_map.find(next.type) != fn_curly_map.end()) ||
				(function_map.find(fn.type) != function_map.end() && next.type == Token::LPARENT)
				) {
				string function_name = (fn.type == Token::LCURLY) ? fn_curly_map.at(next.type) : function_map.at(fn.type);
				Token::Type functiontype = ((fn.type == Token::LCURLY) ? next.type : fn.type);
				fn = lex.GoNext();

				Token::Type splitter_token1 = (function_map_splitter.find(functiontype) != function_map_splitter.end()) ? 
					function_map_splitter.at(functiontype) : Token::COMMA;
				while (params.empty() || fn.type == splitter_token1) {	//循环与递归取参数	
					bool _inFuncion = (function_name != "");
					params.push_back(processFunc(seq, lex, _inFuncion, splitter_token1, needLoop));
					fn = lex.ReadCurrent();
				}

				result += processFuncName(function_name, functiontype, params);
			}
			else {															//按顺序遍历字串
				if (fn.type == Token::LPARENT || fn.type == Token::LCURLY) {
					level++;
					result += word;
				}
				else if (fn.type == Token::RPARENT || fn.type == Token::RCURLY) {
					level--;

					if (level < 0) {//所有参数结束，返回上层
									//result += word;
						return result;
					}
					result += (fn.type == Token::RCURLY) ? ")" : word;

				}
				//else if (  (fn.type == Token::COMMA)) {
				else if (inFuncion && level == 0 && (fn.type == splitter_token)) {
					level--;
					//cout << inFuncion << endl;
					if (level < 0)
						return result;
					result += word;
				}
				else {
					result += word;
				}

			}
			fn = lex.GoNext();
		} while (fn.type != Token::EOS);

		return result;
	}

} // namespace


std::string replaceEscapeSequences(const std::string & query) {
	//增加SAMPLE
	list<std::string> findstr;
	findstr.push_back("SELECT SUM({fn CONVERT((CASE WHEN (`白坭所配变运行量测数`");
	findstr.push_back("SUM({fn CONVERT((CASE WHEN(`白坭所配变运行量测数`");
	findstr.push_back("HAVING(NOT(SUM(`白坭所配变运行量测数`.`P`) IS NULL))");

	string repPosStr = "WHERE(((`白坭所配变运行量测数`";

	string addstr = " SAMPLE 0.1 ";
	map<string, list<string>> mfindstr;
	map<string, string> mrepstr;

	mfindstr[repPosStr] = findstr;
	mrepstr[repPosStr] = addstr;

	string s = query;
	s = AddSample(s, mfindstr, mrepstr);

	//LOGQUERY("\r\n AddSample::\r\n" + s);

	s = ReplaceDataTime(s);

	const char * p = s.c_str();
	const char * end = p + s.size();
	const char * st = p;
	int level = 0;
	std::string result = "";
	

	//ret += processEscapeSequences(StringView(st, p + 1));
	const StringView seq = StringView(st, end);
	Lexer lex(seq);
	lex.SetEmitSpaces(true);
	result = processFunc(seq, lex, false, Token::COMMA, true);

	//LOGQUERY("\r\n processFunc::\r\n" + result);
	return result;
}
