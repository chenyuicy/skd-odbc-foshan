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
	DECLARE(COALESCE),
	DECLARE(SPLIT),
	DECLARE(LEFT),
    DECLARE(CONVERT),
	DECLARE(LTRIM),
	DECLARE(EXTRACT),
	DECLARE(CAST),
	DECLARE(FROM),
	DECLARE(AS),
	DECLARE(DECODE),
    DECLARE(ROUND),
	DECLARE(FLOOR),
    DECLARE(POWER),
	DECLARE(SIGN),
    DECLARE(SQRT),
    DECLARE(ABS),
    DECLARE(TRUNCATE),
    DECLARE(TIMESTAMPDIFF),
    DECLARE(TIMESTAMPADD),
    DECLARE(CURRENT_TIMESTAMP),
    DECLARE(CURDATE),
	DECLARE(DAYOFYEAR),
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

Lexer::Lexer(const StringView text) : text_(text), emit_space_(false),current_({ Token::INVALID, StringView() }), cur_(text.data()), end_(text.data() + text.size()) {}

Token Lexer::GoNext() {
    if (!readed_.empty()) {
        //const Token token(readed_.front());
		current_ = readed_.front();
        readed_.pop_front();
        return current_;
    }
	current_ = NextToken();
    return current_;
}

Token Lexer::GoNext(Token::Type expected){
    if (readed_.empty()) {
        readed_.push_back(NextToken());
    }

    if (readed_.front().type == expected) {
        //const Token token(readed_.front());
		current_ = readed_.front();
        readed_.pop_front();
        return current_;
    }

    return Token{Token::INVALID, StringView()};
}

Token Lexer::ReadCurrent() {
	return current_;
}

Token Lexer::ReadAhead(size_t n) {
	if (n <= 0)return MakeToken(Token::INVALID, 1);
    while (readed_.size() < n ) {
        readed_.push_back(NextToken());
    }

    return readed_[n-1];
}

bool Lexer::Match(Token::Type expected) {
    if (readed_.empty()) {
        readed_.push_back(NextToken());
    }

    if (readed_.front().type != expected) {
        return false;
    }

    GoNext();
    return true;
}


Token Lexer::MakeToken(const Token::Type type, size_t len) {
    const Token token{type, StringView(cur_, len)};

    for (; len > 0; --len) {
        ++cur_;
    }

    return token;
}

Token Lexer::ReadNext() {
    return ReadAhead(1);
}

void Lexer::SetEmitSpaces(bool value) {
    emit_space_ = value;
}

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
			if (emit_space_)
				return MakeToken(Token::SPACE, 1);
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
		case '*':
			return MakeToken(Token::MULTI, 1);
		case '/':
			return MakeToken(Token::DIV, 1);
		case '+':
			return MakeToken(Token::PLUS, 1);
		//case '-':
		//	return MakeToken(Token::DIV, 1);

		case '\'': {
			const char * st = cur_;
			bool has_slash = false;

			for (++cur_; cur_ < end_; ++cur_) {
				if (*cur_ == '\\' && !has_slash) {
					has_slash = true;
					continue;
				}
				if (*cur_ == '\'' && !has_slash) {
					return Token{ Token::STRING, StringView(st, ++cur_) };
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
						else if (!inside_quotes)
							return Token{ Token::IDENT, StringView(st, ++cur_) };
						if (cur_ < end_)
							++cur_;
					}
					/*if (!isalpha(*cur_) && !isdigit(*cur_) && *cur_ != '_' && *cur_ != '.') {
						return Token{ Token::INVALID, StringView(st, cur_) };
					}*/
				}

				break;
			}

			if (isalpha(*cur_) || *cur_ == '_') {
				for (++cur_; cur_ < end_; ++cur_) {
					if (!isalpha(*cur_) && !isdigit(*cur_) && *cur_ != '_' && *cur_ != '.') {
						break;
					}
				}

				return Token{ LookupIdent(to_upper(StringView(st, cur_))), StringView(st, cur_) };
			}

			if (isdigit(*cur_) || *cur_ == '.' || *cur_ == '-') {
				bool has_dot = *cur_ == '.';
				bool has_minus = *cur_ == '-';

				for (++cur_; cur_ < end_; ++cur_) {
					if (*cur_ == '.') {
						if (has_dot) {
							return Token{ Token::INVALID, StringView(st, cur_) };
						}
						else {
							has_dot = true;
						}
						continue;
					}
					else if (*cur_ == '-') {
						if (has_minus) {
							return Token{ Token::INVALID, StringView(st, cur_) };
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

			return Token{ Token::OTHER, StringView(st, cur_) };
		}
		}
	}

	return Token{ Token::EOS, StringView() };
}