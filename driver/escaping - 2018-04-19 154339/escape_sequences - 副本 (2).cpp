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

using namespace std;

namespace {

	const std::map<const Token::Type, const std::string> fn_curly_map{
		{ Token::FN, "" },
		{ Token::TS, "toDateTime" },
		{ Token::D , "toDate" }
	};

const std::map<const std::string, const std::string> fn_convert_map{
    {"SQL_TINYINT", "toUInt8"},
    {"SQL_SMALLINT", "toUInt16"},
    {"SQL_INTEGER", "toInt32"},
    {"SQL_BIGINT", "toInt64"},
    {"SQL_REAL", "toFloat32"},
    {"SQL_DOUBLE", "toFloat64"},
    {"SQL_VARCHAR", "toString"},
    {"SQL_DATE", "toDate"},
    {"SQL_TYPE_DATE", "toDate"},
    {"SQL_TIMESTAMP", "toDateTime"},
    {"SQL_TYPE_TIMESTAMP", "toDateTime"},
};

const std::map<const Token::Type, const std::string> function_map{
    {Token::ROUND, "round"},
	{Token::FLOOR, "floor" },
    {Token::POWER, "pow"},
    {Token::TRUNCATE, "trunc"},
    {Token::SQRT, "sqrt"},
    {Token::ABS, "abs"},
    {Token::CONCAT, "concat"},
    {Token::CURDATE, "today"},
    {Token::TIMESTAMPDIFF, "dateDiff"},
    //{Token::TIMESTAMPADD, "dateAdd" },
};

const std::map<const Token::Type, const std::string> function_map_strip_params{
    {Token::CURRENT_TIMESTAMP, "toUnixTimestamp(now())"},
};

const std::map<const Token::Type, const std::string> literal_map{
    // {Token::SQL_TSI_FRAC_SECOND, ""},
    {Token::SQL_TSI_SECOND,  "'second'"},
    {Token::SQL_TSI_MINUTE,  "'minute'"},
    {Token::SQL_TSI_HOUR,    "'hour'"},
    {Token::SQL_TSI_DAY,     "'day'"},
    {Token::SQL_TSI_WEEK,    "'week'"},
    {Token::SQL_TSI_MONTH,   "'month'"},
    {Token::SQL_TSI_QUARTER, "'quarter'"},
    {Token::SQL_TSI_YEAR,    "'year'"},
};

const std::map<const Token::Type, const std::string> timeadd_func_map{
    // {Token::SQL_TSI_FRAC_SECOND, ""},
    {Token::SQL_TSI_SECOND,  "addSeconds"},
    {Token::SQL_TSI_MINUTE,  "addMinutes"},
    {Token::SQL_TSI_HOUR,    "addHours"},
    {Token::SQL_TSI_DAY,     "addDays"},
    {Token::SQL_TSI_WEEK,    "addWeeks"},
    {Token::SQL_TSI_MONTH,   "addMonths"},
    {Token::SQL_TSI_QUARTER, "addQuarters"},
    {Token::SQL_TSI_YEAR,    "addYears"},
};

//string processFunc(const StringView seq, Lexer & lex);
//string processParam(const StringView seq, Lexer & lex);


string processEscapeSequencesImpl(const StringView seq, Lexer & lex);

//通过函数类型进行函数名转换
string convertFunctionByType(const StringView & typeName) {
    const auto type_name_string = typeName.to_string();
    if (fn_convert_map.find(type_name_string) != fn_convert_map.end())
        return fn_convert_map.at(type_name_string);

    return string();
}

//处理成对小括号
string processParentheses(const StringView seq, Lexer & lex) {
    string result;
    result += lex.Consume().literal.to_string(); // (
	int i = 1;
    while (true) {
        const Token tok(lex.Peek());

        // std::cerr << __FILE__ << ":" << __LINE__ << " : "<< tok.literal.to_string() << " type=" << tok.type << " go\n";
		if (tok.type == Token::LPARENT) {
			i++;
			result += tok.literal.to_string();
			lex.Consume();
		} else if (tok.type == Token::RPARENT) {
			i--;
			result += tok.literal.to_string();
            lex.Consume();
			if (i == 0)break;
        } else if (tok.type == Token::LCURLY) {
            lex.SetEmitSpaces(false);
            result += processEscapeSequencesImpl(seq, lex);
            lex.SetEmitSpaces(true);
        } else if (tok.type == Token::EOS || tok.type == Token::INVALID) {
            break;
        } else {
			string istr = tok.literal.to_string();
            result += istr;
            lex.Consume();
        }
    }

    return result;
}

//处理函数或关键标识
string processIdentOrFunction(const StringView seq, Lexer & lex) {
	string result;
	while (lex.Match(Token::SPACE)) {
		result += " ";
    }
	
	const auto token = lex.Peek();

	std::string jstr = token.literal.to_string();
	LOGQUERY("escape::119:start check== "+ jstr);
	 
	if (token.type == Token::LCURLY) {
		LOGQUERY("escape::120== Token::LCURLY");
        lex.SetEmitSpaces(false);
        result += processEscapeSequencesImpl(seq, lex);
        lex.SetEmitSpaces(true);
    } else if (token.type == Token::IDENT) {
		
		const auto token_next = lex.Peek();
		//if (lex.LookAhead(1).type == Token::LPARENT) { // CAST( ... )
		if (token_next.type == Token::LPARENT) { // CAST( ... )
			std::string istr = token.literal.to_string();
			LOGQUERY("escape::126== Token::IDENT== " + istr);
			result += token.literal.to_string();                                            // func name
			lex.Consume();
			result += processParentheses(seq, lex);
		}
		else {
			
			std::string istr = token.literal.to_string();
			LOGQUERY("escape::145== Token::NUMBER OR IDENT== " + istr);
			result += token.literal.to_string();
			lex.Consume();
			result += processIdentOrFunction(seq, lex);


			/* //rox=========================
			  if (!lex.Match(Token::RPARENT)) {
				return seq.to_string();
			  }else{
			    result +=")";
			  }
			}*/

		}
		
    } else if (token.type == Token::NUMBER) {
		LOGQUERY("escape::131== Token::NUMBER OR IDENT== " + token.literal.to_string());
		result += token.literal.to_string();
        lex.Consume();
    }
	else if (token.type == Token::LPARENT) {
		LOGQUERY("escape::140== Token::IDENT== " + token.literal.to_string());
		result += token.literal.to_string();
		lex.Consume();
		result += processParentheses(seq, lex);
		//result += processIdentOrFunction(seq, lex);
	}
	else if (function_map_strip_params.find(token.type) != function_map_strip_params.end()) {
		LOGQUERY("escape::151== function_map_strip_params== " + function_map_strip_params.at(token.type));
		result += function_map_strip_params.at(token.type);
	}
	else{
		LOGQUERY("escape::138== else");
        return "";
    }
    while (lex.Match(Token::SPACE)) {
		result += " ";
    }
    return result;
}

//处理函数
string processFunction(const StringView seq, Lexer & lex) {
    const Token fn(lex.Consume());
	if (fn.type == Token::SPACE) {
		//lex.Consume();
		return " " + processFunction(seq, lex);
	}
    else if (fn.type == Token::CONVERT) {
        string result;
		if (!lex.Match(Token::LPARENT)) {
			return seq.to_string();
		}

		LOGQUERY("escape:processFunction:145:" + seq.to_string());
        auto num = processIdentOrFunction(seq, lex);
		if (num.empty()) {
			return seq.to_string();
		}

            
        result += num;
		LOGQUERY("escape:processFunction:168: "+result);

        while (lex.Match(Token::SPACE)) {
			result += " ";
        }


		//去掉 space ，改为 emitspace false ，while ==> if  rox=============================
		while (lex.Match(Token::RPARENT)) {
			result += ")";
		}

        if (!lex.Match(Token::COMMA)) {
			LOGQUERY("escape:processFunction:175: " + seq.to_string());
			return seq.to_string();
        }

		//去掉 space ，改为 emitspace false ，while ==> if  rox=============================
		while (lex.Match(Token::SPACE)) {
			result += " ";
		}

		while (lex.Match(Token::RPARENT)) {
			result += ")";
		}

		LOGQUERY("escape:processFunction:175: " + result);
        Token type = lex.Consume();
        if (type.type != Token::IDENT) {
            return seq.to_string();
        }
		LOGQUERY("escape:processFunction:180: " + result);
        string func = convertFunctionByType(type.literal.to_string());
        if (!func.empty()) {
            while (lex.Match(Token::SPACE)) {
				result += " ";
            }

			
            if (!lex.Match(Token::RPARENT)) {
                return seq.to_string();
            }
            result = func + "(" + result + ")";
			//round(UB)  floor(round(UB)) =====================================================
        }
        return result;

    } 
	else if (fn.type == Token::TIMESTAMPADD) {
		LOGQUERY("\r\nrox:processFunction:187: Token::TIMESTAMPADD");
        string result;
        if (!lex.Match(Token::LPARENT))
            return seq.to_string();

        Token type = lex.Consume();
        if (timeadd_func_map.find(type.type) == timeadd_func_map.end())
            return seq.to_string();
        string func = timeadd_func_map.at(type.type);
        if (!lex.Match(Token::COMMA))
            return seq.to_string();
        auto ramount = processIdentOrFunction(seq, lex);
        if (ramount.empty())
            return seq.to_string();

        while (lex.Match(Token::SPACE)) {
			result += " ";
        }

        if (!lex.Match(Token::COMMA))
            return seq.to_string();


        auto rdate = processIdentOrFunction(seq, lex);
        if (rdate.empty())
            return seq.to_string();

        if (!func.empty()) {
            while (lex.Match(Token::SPACE)) {
				result += " ";
            }
            if (!lex.Match(Token::RPARENT)) {
                return seq.to_string();
            }
            result = func + "(" + rdate + ", " + ramount + ")";
        }
        return result;
    } 
	else if (function_map.find(fn.type) != function_map.end()) {
		string result = function_map.at(fn.type);
		LOGQUERY("escape:processFunction:237:" + result);
        while (true) {
            const Token tok(lex.Peek());

            if (tok.type == Token::RCURLY) {
				lex.Consume();
                break;
            } else if (tok.type == Token::LCURLY) {
                lex.SetEmitSpaces(false);
                result += processEscapeSequencesImpl(seq, lex);
                lex.SetEmitSpaces(true);
            } else if (tok.type == Token::EOS || tok.type == Token::INVALID) {
                break;
            } else {
                if (literal_map.find(tok.type) != literal_map.end()) {
                    result += literal_map.at(tok.type);
                } else
                    result += tok.literal.to_string();
                lex.Consume();
            }
        }

        return result;

    } 
	else if (function_map_strip_params.find(fn.type) != function_map_strip_params.end()) {
        string result = function_map_strip_params.at(fn.type);
        if (lex.Peek().type == Token::LPARENT) {
            processParentheses(seq, lex); // ignore anything inside ( )
        }

        return result;
    }

    return seq.to_string();
}

//处理日期--年月日YYYY-MM-DD
string processDate(const StringView seq, Lexer & lex) {
	lex.SetEmitSpaces(false);
    Token data = lex.Consume(Token::STRING);
	lex.SetEmitSpaces(true);
    if (data.isInvalid()) {
        return seq.to_string();
    } else {
		/**rox start*/
		string d = data.literal.to_string();
		size_t pos = d.find("-00");
		if (pos != d.npos) {
			return string("toDate(") + d.replace(pos, 3, string("-")) + ")";      
		}
		else {
			return string("toDate(") + d + ")";
		}
		/**rox end*/
		//return string("toDate(") + data.literal.to_string() + ")";
    }
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
        } else {
            if (dot) {
                return string(begin, dot) + (quoted ? "'" : "");
            }
            return token.to_string();
        }
    }

    return token.to_string();
}

//处理日期--时间YYYY-MM-DD HH-NN-SS
string processDateTime(const StringView seq, Lexer & lex) {
	lex.SetEmitSpaces(false);
    Token data = lex.Consume(Token::STRING);
	lex.SetEmitSpaces(true);
    if (data.isInvalid()) {
        return seq.to_string();
    } else {
		/**rox start*/
		string d = data.literal.to_string();
		size_t pos = d.find("-00");
		if (pos != d.npos) {
			return string("toDateTime(") + removeMilliseconds(d.replace(pos, 3, string("-"))) + ")";
		}
		else {
			return string("toDateTime(") + removeMilliseconds(d) + ")";
		}
		/**rox end*/
        //return string("toDateTime(") + removeMilliseconds(data.literal) + ")";
    }
}

//处理转义补充
string processEscapeSequencesImpl(const StringView seq, Lexer & lex) {
    string result;
	string iret;
    if (!lex.Match(Token::LCURLY)) {
        return seq.to_string();
    }

    while (true) {
        while (lex.Match(Token::SPACE)) {
			result += " ";
        }
		lex.SetEmitSpaces(false);
        const Token tok(lex.Consume());
		lex.SetEmitSpaces(true);

        switch (tok.type) {
            case Token::FN:
				//lex.SetEmitSpaces(false);
				iret = processFunction(seq, lex);

				//iret=round(UB)  ======================================================================
				//LOGQUERY("\r\nrox:processEscapeSequencesImpl:337: " + iret);
                result += iret;

				//result =floor(round(UB) =============================================================
				//lex.SetEmitSpaces(true);
                break;

            case Token::D:
                result += processDate(seq, lex);
                break;
            case Token::TS:
                result += processDateTime(seq, lex);
                break;

            // End of escape sequence
            case Token::RCURLY:
				lex.SetEmitSpaces(true);
                return result;
			case Token::LPARENT:
				result += processFunction(seq, lex);
				return result;
			case Token::RPARENT:
				result += ")";
				return result;
			case Token::EOS:
				return result;
            // Unimplemented
            case Token::T:
            default:
                return seq.to_string();
        }
    };
}

//处理转义
string processEscapeSequences(const StringView seq) {
    Lexer lex(seq);
	lex.SetEmitSpaces(true);
    return processEscapeSequencesImpl(seq, lex);
}

string processFunc(const StringView seq, Lexer & lex, int exeuteLevel) 
{
	lex.SetEmitSpaces(true);
	string result = "";
	Token fn= lex.Consume();
	string params[] = { "" };
	int i=0;
	int level = 0;
	string word = "";
	if (fn.type == Token::EOS)
	{
		return result;
	}

	do{
		word = fn.literal.to_string();
		const Token	nexttoken = lex.LookAhead(0);

		/*if (fn.type == Token::LCURLY && nexttoken.type == Token::D)
		{
			lex.Consume();
			//result += "toDate(";
			result += processDate(seq, lex);
			++level;
		}
		else if (fn.type == Token::LCURLY && nexttoken.type == Token::TS)
		{	
			lex.Consume();
			//result += "toDateTime(";
			result += processDateTime(seq, lex);
			++level;
		}
		else*/ if (fn.type == Token::LCURLY && nexttoken.type == Token::FN)
		{
			fn = lex.Consume();
			++level;
			//while (lex.Match(Token::SPACE)) {}
		}
		
		else if ((fn.type == Token::LCURLY && fn_curly_map.find(nexttoken.type) != fn_curly_map.end()) ||
			(function_map.find(fn.type) != function_map.end() && nexttoken.type == Token::LPARENT))
		{
			//function_name = word;
			string function_name = (fn.type == Token::LCURLY)?fn_curly_map.at(nexttoken.type):function_map.at(fn.type);
			//string function_name = function_map.at(fn.type);
			int param_count = 0;
			if (nexttoken.type != Token::LPARENT)
			{
				fn = lex.Consume();
			}
			
		
			//while (fn.type==Token::LPARENT || fn.type==Token::COMMA || fn.type == Token::AS) {	//循环与递归取参数
			while (param_count==0 || fn.type == Token::COMMA || fn.type == Token::AS) {	//循环与递归取参数	
				params[param_count]= processFunc(seq, lex, ++exeuteLevel);
				--exeuteLevel;
				param_count++;
				fn = lex.Consume();
			}
		
			result += function_name;
			for (int i = 0; i < param_count; i++) {
				if (i != 0)
					result += ",";
				result += params[i];
			}
			//result += ")";
			level++;
		}
		/*else if (fn.type == Token::LCURLY && nexttoken.type == Token::FN)
		{
			level++;
			fn = lex.Consume();
		}*/
		else {//按顺序遍历字串
			if (fn.type == Token::LPARENT || fn.type == Token::LCURLY) {
				level++;
				result += (fn.type == Token::LPARENT)?"(":"";
			}
			else if (fn.type == Token::RPARENT || fn.type == Token::RCURLY) {
				level--;
				word = (fn.type == Token::RPARENT ? ")" : "");
				result += word;
				if (level < 0) {//所有参数结束，返回上层
					return result;
				}
				
				if (level == 0 && exeuteLevel > 0)
				{//回上层去处理
					return result;
				}
			}
			else if (level==0 && (fn.type == Token::COMMA || fn.type == Token::AS) ) {	//
				result += word;
				if(exeuteLevel>0)
					return result;
			}
			else {
				result += word;
			}
			
		}
		fn = lex.Consume();	
		
	}while (fn.type != Token::EOS);

	return result;
}


} // namespace

std::string replaceEscapeSequences(const std::string & query) {
	const char * p = query.c_str();
	const char * end = p + query.size();
	const char * st = p;
	int level = 0;
	std::string result="";
	//Token lastToken;


	//ret += processEscapeSequences(StringView(st, p + 1));
	const StringView seq = StringView(st, end);
	Lexer lex(seq);
	lex.SetEmitSpaces(true);
	result=processFunc(seq, lex, 0);
	return result;
}

std::string replaceEscapeSequences1(const std::string & query) {
    const char * p = query.c_str();
    const char * end = p + query.size();
    const char * st = p;
    int level = 0;
    std::string ret;

    while (p != end) {
        switch (*p) {
            case '{':
                if (level == 0) {
                    if (st < p) {
						//ret += StringView(st, p);//
						ret += std::string(st, p); 
                    }
                    st = p;
                }
                level++;
                break;

            case '}':
                if (level == 0) {
                    // TODO unexpected '}'
                    return query;
                }
                if (--level == 0) {
					
					ret += processEscapeSequences(StringView(st, p + 1));
					
                    st = p + 1;
                }
                break;
        }

        ++p;
    }

    if (st < p) {
        //ret += StringView(st, p); 
		ret += std::string(st, p);
    }

    return ret;
}


