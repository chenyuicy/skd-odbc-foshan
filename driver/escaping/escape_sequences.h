#pragma once

#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>     
#include <boost/tokenizer.hpp>     
#include <boost/algorithm/string/classification.hpp>
using namespace std;
using namespace boost;

struct date
{
	string year = "";
	string month = "";
	string day = "";
	string hour = "";
	string minute = "";
	string second = "";
};

std::string replaceEscapeSequences(const std::string & query);
