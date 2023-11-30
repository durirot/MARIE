#include "assemble.hpp"

#include "instructions.h"
#include "static_hashtable.hpp"

#include <charconv>
#include <cstddef>
#include <deque>
#include <fmt/core.h>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

enum struct DataType {
    Identifier,
    Literal,
    Word,
};

constexpr Word maxAddressSize()
{
    Word value = 2;
    for (int i = 0; i < 11; i++)
        value *= 2;
    return value;
}

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

    Comma,
    Unknown,

    Eof,
};

bool tokenIsInstruction(Token tok)
{
    return ((int)tok >= (int)Token::Load && (int)tok <= (int)Token::Jump);
}

Instruction tokenToInstruction(Token tok)
{
    if (!tokenIsInstruction(tok))
        throw std::runtime_error("token is not an instruction");

    constexpr int offset = (int)Token::Load - (int)Instruction::Load;

    return (Instruction)((int)tok - offset);
}

bool tokenHasZeroOperands(Token tok)
{
    return ((int)tok >= (int)Token::Input && (int)tok <= (int)Token::Halt);
}

struct Lexer {
    Lexer(std::string_view text);

    std::pair<Token, std::size_t> nextToken();

    std::string_view getPrevString()
    {
        return prevString;
    }

    std::string_view getLine(std::size_t textLocation)
    {
        std::size_t start = textLocation;
        while (textLocation < text.size() && text[textLocation] != '\n' && text[textLocation] != '\0') {
            textLocation++;
        }
        return std::string_view { text.data() + start, text.data() + textLocation };
    }

private:
    std::string_view text;
    std::size_t textLocation;

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

constexpr std::size_t string_view_hash(std::string_view str)
{
    std::size_t hash = 0;
    for (auto c : str) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

static constinit static_hashtable<std::string_view, Token, 30, string_view_hash> keywords({
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

std::pair<Token, std::size_t> Lexer::nextToken()
{
    char c = nextChar();

    while (isWhiteSpace(c)) {
        c = nextChar();
    }

    if (c == '\0') {
        return std::pair(Token::Eof, textLocation);
    }

    if (c == ';') {
        c = nextChar();
        while (c != '\n' && c != '\0') {
            c = nextChar();
        }
        c = nextChar();
    }

    std::size_t startLocation = textLocation - 1;
    if (isAlpha(c)) {
        // fmt::print("{} is alpha\n", c);
        c = nextChar();
        while (isAlphaNum(c)) {
            // fmt::print("{} is alpha\n", c);
            c = nextChar();
        }
        // fmt::print("{} is not alpha\n", c);
        if (textLocation < text.size()) {
            textLocation -= 1;
        }

        prevString = std::string_view { text.data() + startLocation, text.data() + textLocation };
        // fmt::print("prev string [{}]\n", prevString);

        auto result = keywords.get(prevString);
        // result will be 0 (aka Token::Label) if not found
        return std::pair(result, startLocation);
    }

    if (isNum(c)) {
        if (c == '0') {
            c = nextChar();
            if (c == 'x') {
                c = nextChar();
            } else {
                throw std::runtime_error(fmt::format("expected a hexidecimal value (x), instead got {}", c));
            }
        } else {
            throw std::runtime_error(fmt::format("expected a hexidecimal value (0), instead got {}", c));
        }
        std::size_t startLocation = textLocation - 1;

        if (!isNum(c)) {
            throw std::runtime_error(fmt::format("expected a number after 0x instead got {}", c));
        }

        while (isNum(c)) {
            c = nextChar();
        }
        if (textLocation < text.size()) {
            textLocation -= 1;
        }

        prevString = std::string_view { text.data() + startLocation, text.data() + textLocation };

        return std::pair(Token::Number, startLocation);
    }

    if (c == ',') {
        return std::pair(Token::Comma, startLocation);
    }

    throw std::runtime_error(fmt::format("[lexer error] unexpected token [{}]", int(c)));
    // return std::pair(Token::Unknown, startLocation);
}

std::vector<Word> assembleFromFile(const char* input)
{
    FILE* file = fopen(input, "r");
    fseek(file, 0, SEEK_END);
    std::size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*)malloc(size * sizeof(char));
    if (data == nullptr) {
        throw std::runtime_error("failed to allocate memory??? how!");
    }
    fread(data, 1, size, file);
    fclose(file);

    auto result = assembleFromText(data);
    free(data);
    return result;
}

std::vector<Word> assembleFromText(const char* input)
{
    Lexer lex(input);

    std::unordered_map<std::string_view, Word> labels;
    std::deque<InstructionData> instructions;

    // pass 1
    std::pair<Token, std::size_t> token = { Token::Unknown, 0 };
    Word pos = 0;
    while (true) {
        token = lex.nextToken();
        if (token.first == Token::Eof) {
            break;
        }

        if (token.first == Token::Label) {
            auto errorString = lex.getPrevString();
            auto errorLocation = token.second;
            token = lex.nextToken();

            if (token.first == Token::Comma) {
                labels[lex.getPrevString()] = pos;
            } else {
                throw std::runtime_error(fmt::format("on:\n{}\nlabel missing comma {}", lex.getLine(errorLocation), errorString));
            }
            token = lex.nextToken();
        }

        if (tokenIsInstruction(token.first)) {
            if (tokenHasZeroOperands(token.first)) {
                // fmt::print("token has zero operands\n");
                instructions.push_back(InstructionData {
                    .instr = tokenToInstruction(token.first),
                    .dataType = DataType::Literal,
                    .literal = 0 });
                pos++;
            } else {
                // get literal or label
                auto operands = lex.nextToken();

                // assert its either a literal or label
                if (operands.first == Token::Label) {
                    instructions.push_back(InstructionData {
                        .instr = tokenToInstruction(token.first),
                        .dataType = DataType::Identifier,
                        .identifier = lex.getPrevString() });
                    pos++;
                } else if (operands.first == Token::Number) {
                    auto prevString = lex.getPrevString();
                    Word value;
                    (void)std::from_chars(prevString.data(), prevString.data() + prevString.length(), value, 16);
                    if (value >= maxAddressSize()) {
                        throw std::runtime_error(fmt::format("[parser error] on:\n{}\noperand outside of max word range (2^12)",
                            lex.getLine(operands.second)));
                    }
                    instructions.push_back({
                        .instr = tokenToInstruction(token.first),
                        .dataType = DataType::Literal,
                        .literal = value,
                    });
                    pos++;
                } else {
                    throw std::runtime_error(fmt::format("[parser error] on:\n{}\ninvalid operand {}",
                        lex.getLine(operands.second), lex.getPrevString()));
                }
            }
        } else if (token.first == Token::Number) {
            auto prevString = lex.getPrevString();
            Word value;
            (void)std::from_chars(prevString.data(), prevString.data() + prevString.length(), value, 16);
            instructions.push_back({
                .dataType = DataType::Word,
                .literal = value,
            });
            pos++;
        } else {
            throw std::runtime_error(fmt::format("[parser error] on:\n{}\n unexpected token {}",
                lex.getLine(token.second), (int)token.first));
        }
    }

    // pass 2
    std::vector<Word> binaryInstructions;
    binaryInstructions.reserve(instructions.size());

    for (auto& instr : instructions) {
        switch (instr.dataType) {
        case DataType::Word: {
            binaryInstructions.push_back(instr.literal);
            break;
        }
        case DataType::Identifier: {
            Word instruction = (Word)instr.instr;
            instruction = instruction << 12;
            instruction |= (labels[instr.identifier] & 0x0fff);
            binaryInstructions.push_back(instruction);
            break;
        }
        case DataType::Literal: {
            Word instruction = (Word)instr.instr;
            instruction = instruction << 12;
            instruction |= (instr.literal & 0x0fff);
            binaryInstructions.push_back(instruction);
            break;
        }
        default:
            throw std::runtime_error(fmt::format("cannot interpret DataType (eof or unknown) {}", (int)instr.dataType));
        }
    }

    return binaryInstructions;
}
