#include "assemble.h"

#include "instructions.h"
#include "static_hashtable.hpp"

#include <string_view>

enum struct DataType {
    Identifier,
    Word,
    None,
    Eof,
};

struct InstructionData {
    Instruction instr;
    DataType dataType;
    union {
        std::string_view identifier;
        Word literal;
    };
};

enum struct Token {
    Label,
    Number,

    Load,
    Store,
    Add,
    Subt,
    Input,
    Output,
    Halt,
    Skipcond,
    Jump,

    Eof,
};

struct Lexer {
    Lexer(std::string_view text);

    std::pair<Token, size_t> nextToken();

private:
    std::string_view text;
    size_t textLocation;

	std::string_view prevString;

    const char nextChar()
    {
        if (textLocation >= text.size()) {
            return '\0';
        }
        return *(text.data() + textLocation++);
    }

    static constexpr bool isNum(const char c)
    {
        return c >= '0' && c <= '9';
    }

    static constexpr bool isAlpha(const char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static constexpr bool isAlphaNum(const char c)
    {
        return isNum(c) || isAlpha(c);
    }

    static constexpr bool isWhiteSpace(const char c)
    {
        return c == '\n' || c == '\r' || c == '\t' || c == ' ';
    }
};

inline constexpr auto string_view_hash(std::string_view str) -> std::size_t
{
    std::size_t hash = 0;
    for (auto c : str) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

static constexpr static_hashtable<std::string_view, Token, 20, string_view_hash> keywords({
    { "load", Token::Load },
    { "store", Token::Store },
    { "add", Token::Add },
    { "subt", Token::Subt },
    { "input", Token::Input },
    { "output", Token::Output },
    { "halt", Token::Halt },
    { "skipcond", Token::Skipcond },
    { "jump", Token::Jump },
});

Lexer::Lexer(std::string_view text)
    : text(text)
    , textLocation(0)
{
}

std::pair<Token, size_t> Lexer::nextToken()
{
    char c = nextChar();

    if (c == '\0') {
        return std::pair(Token::Eof, textLocation);
    }

    while (isWhiteSpace(c)) {
        c = nextChar();
    }

    size_t startLocation = textLocation - 1;
    if (isAlpha(c)) {
        while (isAlphaNum(c = nextChar())) { }
		
		prevString = std::string_view{text.data()+startLocation, textLocation};

		auto result = keywords.get(prevString);
		// result will be 0 or Token::Label if not found
		return std::pair(result, startLocation);
    }

	if (isNum(c)) {
		if (c == 0) {
			c = nextChar();
			if (c == 'x') {}
		}

		while (isNum(c = nextChar())) {}
		prevString = std::string_view{text.data()+startLocation, textLocation};

		return std::pair(Token::Number, startLocation);
	}

	return std::pair(Token::Eof, 0);
}
