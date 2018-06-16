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

//�ڹؼ����б��в���ident
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

//��ȡ��һ����־
Token Lexer::Consume() {
    if (!readed_.empty()) {
        const Token token(readed_.front());
        readed_.pop_front();
        return token;
    }

    return NextToken();
}

//��ȡ��һ��Ԥ��expected���͵ı�ʶ
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

//����n��char
Token Lexer::LookAhead(size_t n) {
    while (readed_.size() < n + 1) {
        readed_.push_back(NextToken());
    }

    return readed_[n];
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
    return LookAhead(0);
}


//�ո���-true������ո� false�ո����
void Lexer::SetEmitSpaces(bool value) {
    emit_space_ = value;
}

//��ȡ��һ����ʶ�򵥴�
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
					LOGQUERY("lexer:147:" );
					return MakeToken(Token::SPACE, 1);
				}
                    
                continue;

                /** Delimiters */

            case '(':
				LOGQUERY("lexer::NextToken:156: ( ");
                return MakeToken(Token::LPARENT, 1);
            case ')':
				LOGQUERY("lexer::NextToken:159: ) ");
                return MakeToken(Token::RPARENT, 1);
            case '{':
				LOGQUERY("lexer::NextToken:162: { ");
                return MakeToken(Token::LCURLY, 1);
            case '}':
				LOGQUERY("lexer::NextToken:165: } ");
                return MakeToken(Token::RCURLY, 1);
            case ',':
				LOGQUERY("lexer::NextToken:168: , ");
                return MakeToken(Token::COMMA, 1);

            case '\'': {
				LOGQUERY("lexer:172: quota ");
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
				LOGQUERY("lexer::NextToken:enter Default:192:");
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
					
					//LOGQUERY("lexer::NextToken:215::"+std::string(st, cur_));
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
					//LOGQUERY("lexer::NextToken253::" + std::string(st, cur_));
                    return Token{Token::NUMBER, StringView(st, cur_)};
                }

                for (; cur_ < end_; ++cur_) {
                    if (isspace(*cur_)) {
                        break;
                    }
                }
				//LOGQUERY("::NextToken262::" + std::string(st, cur_));
                return Token{Token::OTHER, StringView(st, cur_)};
            }
        }
    }

    return Token{Token::EOS, StringView()};
}
