#include "lexer.h"
#include "../log.h"
#include <algorithm>
#include <unordered_map>

namespace {

#define DECLARE(NAME) \
    { #NAME, Token::NAME }
#define DECLARE_SQL_TSI(NAME) \
    { #NAME, Token::SQL_TSI_##NAME }

static const std::unordered_map<std::string, Token::Type> KEYWORDS = {
    DECLARE(FN),
    DECLARE(D),
    DECLARE(T),
    DECLARE(TS),
	DECLARE(AS),
    DECLARE(CONCAT),
    DECLARE(CONVERT),
    DECLARE(ROUND),
	DECLARE(FLOOR),
    DECLARE(POWER),
    DECLARE(SQRT),
    DECLARE(ABS),
    DECLARE(TRUNCATE),
    DECLARE(TIMESTAMPDIFF),
    DECLARE(TIMESTAMPADD),
    DECLARE(CURRENT_TIMESTAMP),
    DECLARE(CURDATE),
    //DECLARE(SQL_TSI_FRAC_SECOND),
    DECLARE(SQL_TSI_SECOND),
    DECLARE(SQL_TSI_MINUTE),
    DECLARE(SQL_TSI_HOUR),
    DECLARE(SQL_TSI_DAY),
    DECLARE(SQL_TSI_WEEK),
    DECLARE(SQL_TSI_MONTH),
    DECLARE(SQL_TSI_QUARTER),
    DECLARE(SQL_TSI_YEAR),

    // DECLARE_SQL_TSI(MILLISECOND),
    DECLARE_SQL_TSI(SECOND),
    DECLARE_SQL_TSI(MINUTE),
    DECLARE_SQL_TSI(HOUR),
    DECLARE_SQL_TSI(DAY),
    //DECLARE_SQL_TSI(DAYOFYEAR),
    DECLARE_SQL_TSI(WEEK),
    DECLARE_SQL_TSI(MONTH),
    DECLARE_SQL_TSI(QUARTER),
    DECLARE_SQL_TSI(YEAR),
};
#undef DECLARE
#undef DECLARE_SQL_TSI

//在关键字列表中查找ident
static Token::Type LookupIdent(const std::string & ident) {
    auto ki = KEYWORDS.find(ident);
    if (ki != KEYWORDS.end()) {
        return ki->second;
    }
    return Token::IDENT;
}
}

std::string to_upper(const StringView & str) {
    std::string ret(str.data(), str.size());
    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
    return ret;
}

Lexer::Lexer(const StringView text) : text_(text), emit_space_(false), cur_(text.data()), end_(text.data() + text.size()) {}

//读取下一个标志
Token Lexer::Consume() {
    if (!readed_.empty()) {
        const Token token(readed_.front());
        readed_.pop_front();
        return token;
    }

    return NextToken();
}

//读取下一个预期expected类型的标识
Token Lexer::Consume(Token::Type expected) {
    if (readed_.empty()) {
        readed_.push_back(NextToken());
    }

    if (readed_.front().type == expected) {
        const Token token(readed_.front());
        readed_.pop_front();
        return token;
    }

    return Token{Token::INVALID, StringView()};
}

//读第n个char
Token Lexer::GoAhead(size_t n) 
{
    while (readed_.size() < n + 1) 
	{
        readed_.push_back(NextToken());
    }

    return readed_[n];
}

//TODO::need modify
Token Lexer::LookAhead(size_t n)
{
	Token pn;
	while (n)
	{
		pn = GetNextToken();
		--n;
	}
	return pn;
}

bool Lexer::Match(Token::Type expected) {
    if (readed_.empty()) {
        readed_.push_back(NextToken());
    }

    if (readed_.front().type != expected) {
        return false;
    }

    Consume();
    return true;
}


Token Lexer::MakeToken(const Token::Type type, size_t len) {
    const Token token{type, StringView(cur_, len)};

    for (; len > 0; --len) {
        ++cur_;
    }

    return token;
}

Token Lexer::Peek() {
    return GoAhead(0);
}


//空格处理-true则输出空格 false空格忽略
void Lexer::SetEmitSpaces(bool value) {
    emit_space_ = value;
}

//读取下一个标识或单词 lexer不前进
Token Lexer::GetNextToken()
{
	const char* icur_ = cur_;
	for (; cur_ < end_; ++cur_) {
		switch (*cur_) {
			/** Whitespaces */
		case '\0':
		case ' ':
		case '\t':
		case '\f':
		case '\n':
		case '\r':
			if (emit_space_) {
				cur_ = icur_;
				return Token{ Token::SPACE, StringView(icur_)};
			}

			continue;

			/** Delimiters */

		case '(':
			cur_ = icur_;
			return Token{ Token::LPARENT, StringView(icur_) };
		case ')':
			cur_ = icur_;
			return Token{ Token::RPARENT, StringView(icur_) };
		case '{':
			cur_ = icur_;
			return Token{ Token::LCURLY, StringView(icur_) };
		case '}':
			cur_ = icur_;
			return Token{ Token::RCURLY, StringView(icur_) };
		case ',':
			cur_ = icur_;
			return Token{ Token::COMMA, StringView(icur_) };

		case '\'': {
			const char * st = cur_;
			bool has_slash = false;

			for (++cur_; cur_ < end_; ++cur_) {
				if (*cur_ == '\\' && !has_slash) {
					//has_slash = true;//rox
					continue;
				}
				if (*cur_ == '\'' && !has_slash) {					
					StringView istr = StringView(st, ++cur_);
					cur_ = icur_;
					return Token{ Token::STRING, istr };
				}

				has_slash = false;
			}
			return Token{ Token::INVALID, StringView(st, cur_ - st) };
		}

		default: {
			const char * st = cur_;

			if (*cur_ == '`') {
				bool inside_quotes = true;
				for (++cur_; cur_ < end_; ++cur_) {
					if (*cur_ == '`') {
						inside_quotes = !inside_quotes;
						if (cur_ < end_ && *(cur_ + 1) == '.') {
							++cur_;
							continue;
						}
						else if (!inside_quotes) {
							//std::string istr = std::string(st, ++cur_);
							//LOGQUERY("lexer:194::" + istr);
							//break;
							//return Token{ Token::IDENT, istr };
							

							cur_ = icur_;
							return Token{ Token::IDENT, StringView("") };
						}
						else if (cur_ < end_)
							++cur_;
					}
					//if (!(*cur_ >= 0x4e00 && *cur_ <= 0x9fbb) && !isalpha(*cur_) && !isdigit(*cur_) && *cur_ != '_' && *cur_ != '.') {
					//    return Token{Token::INVALID, StringView(st, cur_)};
					//}
				}


				break;
			}

			if (isalpha(*cur_) || *cur_ == '_') {
				for (++cur_; cur_ < end_; ++cur_) {
					if (!isalpha(*cur_) && !isdigit(*cur_) && *cur_ != '_' && *cur_ != '.') {
						break;
					}
				}

				Token fn;
				fn.type = LookupIdent(to_upper(StringView(st, cur_)));
				cur_ = icur_;
				return Token{ fn.type, StringView("") };
			}

			if (isdigit(*cur_) || *cur_ == '.' || *cur_ == '-') {
				bool has_dot = *cur_ == '.';
				bool has_minus = *cur_ == '-';

				for (++cur_; cur_ < end_; ++cur_) {
					if (*cur_ == '.') {
						if (has_dot) {
							cur_ = icur_;
							return Token{ Token::INVALID, StringView("") };
						}
						else {
							has_dot = true;
						}
						continue;
					}
					else if (*cur_ == '-') {
						if (has_minus) {
							cur_ = icur_;
							return Token{ Token::INVALID, StringView("") };
						}
					}

					if (!isdigit(*cur_)) {
						break;
					}
				}
				return Token{ Token::NUMBER, StringView(st, cur_) };
			}

			for (; cur_ < end_; ++cur_) {
				if (isspace(*cur_)) {
					break;
				}
			}
			cur_ = icur_;
			return Token{ Token::OTHER, StringView("") };
		}
		}
	}
	cur_ = icur_;
	return Token{ Token::EOS, StringView() };
}

//读取下一个标识或单词 lexer前进
Token Lexer::NextToken() {
	for (; cur_ < end_; ++cur_) {
        switch (*cur_) {
                /** Whitespaces */

            case '\0':
            case ' ':
            case '\t':
            case '\f':
            case '\n':
            case '\r':
				if (emit_space_) {
					return MakeToken(Token::SPACE, 1);
				}
                    
                continue;

                /** Delimiters */

            case '(':
                return MakeToken(Token::LPARENT, 1);
            case ')':
                return MakeToken(Token::RPARENT, 1);
            case '{':
                return MakeToken(Token::LCURLY, 1);
            case '}':
                return MakeToken(Token::RCURLY, 1);
            case ',':
                return MakeToken(Token::COMMA, 1);

            case '\'': {
                const char * st = cur_;
                bool has_slash = false;

                for (++cur_; cur_ < end_; ++cur_) {
                    if (*cur_ == '\\' && !has_slash) {
                        //has_slash = true;//rox
                        continue;
                    }
                    if (*cur_ == '\'' && !has_slash) {
						StringView istr = StringView(st, ++cur_);
                        return Token{Token::STRING, istr };
                    }

                    has_slash = false;
                }
                return Token{Token::INVALID, StringView(st, cur_ - st)};
            }
			
            default: {
                const char * st = cur_;

                if (*cur_ == '`') {
                    bool inside_quotes = true;
                    for (++cur_; cur_ < end_; ++cur_) {
                        if (*cur_ == '`') {
                            inside_quotes = !inside_quotes;
                            if (cur_ < end_ && *(cur_ + 1) == '.') {
                                ++cur_;
                                continue;
							}
							else if (!inside_quotes) {
								//std::string istr = std::string(st, ++cur_);
								//LOGQUERY("lexer:194::" + istr);
								//break;
								//return Token{ Token::IDENT, istr };
								return Token{ Token::IDENT, StringView(st, ++cur_) };
							}
                            else if (cur_ < end_)
                                ++cur_;
                        }
                        //if (!(*cur_ >= 0x4e00 && *cur_ <= 0x9fbb) && !isalpha(*cur_) && !isdigit(*cur_) && *cur_ != '_' && *cur_ != '.') {
                        //    return Token{Token::INVALID, StringView(st, cur_)};
                        //}
                    }
					

                    break;
                }
				
                if ( isalpha(*cur_) || *cur_ == '_') {
                    for (++cur_; cur_ < end_; ++cur_) {
                        if (!isalpha(*cur_) && !isdigit(*cur_) && *cur_ != '_' && *cur_ != '.') {
							break;
                        }
                    }
					
                    return Token{LookupIdent(to_upper(StringView(st, cur_))), StringView(st, cur_)};
                }

                if (isdigit(*cur_) || *cur_ == '.' || *cur_ == '-') {
                    bool has_dot = *cur_ == '.';
                    bool has_minus = *cur_ == '-';

                    for (++cur_; cur_ < end_; ++cur_) {
                        if (*cur_ == '.') {
                            if (has_dot) {
                                return Token{Token::INVALID, StringView(st, cur_)};
                            } else {
                                has_dot = true;
                            }
                            continue;
                        } else if (*cur_ == '-') {
                            if (has_minus) {
                                return Token{Token::INVALID, StringView(st, cur_)};
                            }
                        }

                        if (!isdigit(*cur_)) {
                            break;
                        }
                    }
                    return Token{Token::NUMBER, StringView(st, cur_)};
                }

                for (; cur_ < end_; ++cur_) {
                    if (isspace(*cur_)) {
                        break;
                    }
                }
                return Token{Token::OTHER, StringView(st, cur_)};
            }
        }
    }
    return Token{Token::EOS, StringView()};
}
