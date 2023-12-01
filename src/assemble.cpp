#include "assemble.hpp"

#include "instructions.h"
#include "static_hashtable.hpp"

#include <charconv>
#include <cstddef>
#include <deque>
#include <fmt/core.h>
#include <map>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

bool hasErrors = false;
void reportError(std::string error)
{
    fmt::print("{}\n", error);
    hasErrors = true;
}

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
    std::size_t textLocation;
    union {
        std::string_view identifier;
        Word literal;
    };
};

enum struct Token {
    Label,
    DecNumber,
    HexNumber,

    Jns,
    Load,
    Store,
    Add,
    Subt,
    Input,
    Output,
    Halt,
    Skipcond,
    Jump,
    Clear,
    AddI,
    JumpI,
    LoadI,
    StoreI,

    Comma,
    Unknown,

    Eof,
};

const char* tokenToString(Token tok)
{
    switch (tok) {
    case Token::Label:
        return "Token::Label";
    case Token::DecNumber:
        return "Token::DecNumber";
    case Token::HexNumber:
        return "Token::HexNumber";
    case Token::Jns:
        return "Token::Jns";
    case Token::Load:
        return "Token::Load";
    case Token::Store:
        return "Token::Store";
    case Token::Add:
        return "Token::Add";
    case Token::Subt:
        return "Token::Subt";
    case Token::Input:
        return "Token::Input";
    case Token::Output:
        return "Token::Output";
    case Token::Halt:
        return "Token::Halt";
    case Token::Skipcond:
        return "Token::Skipcond";
    case Token::Jump:
        return "Token::Jump";
    case Token::Clear:
        return "Token::Clear";
    case Token::AddI:
        return "Token::AddI";
    case Token::JumpI:
        return "Token::JumpI";
    case Token::LoadI:
        return "Token::LoadI";
    case Token::StoreI:
        return "Token::StoreI";
    case Token::Comma:
        return "Token::Comma";
    case Token::Unknown:
        return "Token::Unknown";
    case Token::Eof:
        return "Token::Eof";
    default:
        return "Unknown Token";
    }
}

bool tokenIsInstruction(Token tok)
{
    return ((int)tok >= (int)Token::Jns && (int)tok <= (int)Token::StoreI);
}

Instruction tokenToInstruction(Token tok)
{
    if (!tokenIsInstruction(tok)) {
        reportError(fmt::format("token {} is not an instruction", tokenToString(tok)));
    }

    constexpr int offset = (int)Token::Jns - (int)Instruction::Jns;

    return (Instruction)((int)tok - offset);
}

bool tokenHasZeroOperands(Token tok)
{
    return ((int)tok >= (int)Token::Input && (int)tok <= (int)Token::Halt) || tok == Token::Clear;
}

struct Lexer {
    Lexer(std::string_view text);

    std::pair<Token, std::size_t> nextToken();

    std::string_view getPrevString();
    std::pair<std::size_t, std::string_view> getLine(std::size_t textLocation);

private:
    std::string_view text {};
    std::size_t textLocation {};
    std::size_t lineNumber = 1;

    // map of which index a new line is contained
    // text location, line number
    std::map<std::size_t, std::size_t> newLines {};

    std::string_view prevString {};
    static constexpr std::size_t PrevStringLowerBufferSize = 15;
    char prevStringLower[PrevStringLowerBufferSize];

    const char nextChar();
    const char peekChar();
    void consumeChar();

    static constexpr bool isNum(const char c);
    static constexpr bool isAlpha(const char c);
    static constexpr bool isAlphaNum(const char c);
    static constexpr bool isWhiteSpace(const char c);

    void consumeUntilNewline();
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
    { "jns", Token::Jns },
    { "load", Token::Load },
    { "store", Token::Store },
    { "add", Token::Add },
    { "subt", Token::Subt },
    { "input", Token::Input },
    { "output", Token::Output },
    { "halt", Token::Halt },
    { "skipcond", Token::Skipcond },
    { "jump", Token::Jump },
    { "clear", Token::Clear },
    { "addi", Token::AddI },
    { "jumpi", Token::JumpI },
    { "loadi", Token::LoadI },
    { "storei", Token::StoreI },
});

Lexer::Lexer(std::string_view text)
    : text(text)
{
    // insert the first line into the map
    newLines.insert(std::pair(0, 1));
}

std::pair<Token, std::size_t> Lexer::nextToken()
{
    char c;
start:
    c = nextChar();

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
        // need to check for whitespace again
        goto start;
    }

    std::size_t startLocation = textLocation - 1;
    if (isAlpha(c)) {
        c = peekChar();
        while (isAlphaNum(c)) {
            consumeChar();
            c = peekChar();
        }

        prevString = std::string_view(text.data() + startLocation, textLocation - startLocation);
        // fmt::print("prev string [{}]\n", prevString);

        std::size_t lowerLen = std::min(prevString.length(), PrevStringLowerBufferSize);
        for (int i = 0; i < lowerLen; i++) {
            prevStringLower[i] = std::tolower(prevString[i]);
        }

        auto result = keywords.get(std::string_view { prevStringLower, lowerLen });
        // result will be 0 (aka Token::Label) if not found
        return std::pair(result, startLocation);
    }

    if (isNum(c)) {
        bool isHex = false;
        if (c == '0') {
            const char newC = peekChar();
            if ((char)std::tolower(newC) == 'x') {
                consumeChar();
                c = nextChar();
                isHex = true;
            }
        }
        std::size_t startLocation = textLocation - 1;

        if (!isNum(c)) {
            // this has to be hex, correct me if I'm wrong
            auto errorInfo = getLine(textLocation - 1);
            reportError(fmt::format("on line {}\n{}\nexpected a number after 0x instead got {}",
                errorInfo.first,
                errorInfo.second,
                c));
            prevString = "0";
            return std::pair(Token::HexNumber, startLocation);
        }

        c = peekChar();
        while (isNum(c)) {
            consumeChar();
            c = peekChar();
        }

        prevString = std::string_view { text.data() + startLocation, text.data() + textLocation };

        if (isHex) {
            return std::pair(Token::HexNumber, startLocation);
        } else {
            return std::pair(Token::DecNumber, startLocation);
        }
    }

    if (c == ',') {
        return std::pair(Token::Comma, startLocation);
    }
    if (c == ':') {
        return std::pair(Token::Comma, startLocation);
    }

    auto errorInfo = getLine(textLocation);
    reportError(fmt::format("[lexer error] on line {}\n{}\nunexpected character: [{}], [{}]",
        errorInfo.first,
        errorInfo.second,
        c,
        int(c)));
    return std::pair(Token::Unknown, startLocation);
}

std::string_view Lexer::getPrevString()
{
    return prevString;
}

std::pair<size_t, std::string_view> Lexer::getLine(std::size_t textLocation)
{
    auto itr = newLines.upper_bound(textLocation);
    std::size_t start;
    std::size_t lineNum;
    itr--;

    start = itr->first;
    lineNum = itr->second;

    while (textLocation < text.size() && text[textLocation] != '\n' && text[textLocation] != '\0') {
        textLocation++;
    }

    if (textLocation < start) {
        throw std::runtime_error("[lexer error] trying to construct a string_view with a start position after the end position, report this bug please.");
    }

    return {
        lineNum,
        std::string_view { text.data() + start, text.data() + textLocation }
    };
}

const char Lexer::nextChar()
{
    if (textLocation >= text.size()) {
        return '\0';
    }
    char c = *(text.data() + textLocation++);

    if (c == '\n') {
        lineNumber += 1;
        newLines.insert(std::pair(textLocation, lineNumber));
    }

    return c;
}

const char Lexer::peekChar()
{
    if (textLocation >= text.size()) {
        return '\0';
    }
    return *(text.data() + textLocation);
}

void Lexer::consumeChar()
{
    (void)nextChar();
}

constexpr bool Lexer::isNum(const char c)
{
    return c >= '0' && c <= '9';
}

constexpr bool Lexer::isAlpha(const char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

constexpr bool Lexer::isAlphaNum(const char c)
{
    return isNum(c) || isAlpha(c);
}

constexpr bool Lexer::isWhiteSpace(const char c)
{
    return c == '\n' || c == '\r' || c == '\t' || c == ' ';
}

void Lexer::consumeUntilNewline()
{
    char c = nextChar();
    while (true) {
        if (c == '\n' || c == '\0') {
            break;
        }
        c = nextChar();
    }
}

} // anonymous namespace

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
                auto errorInfo = lex.getLine(errorLocation);
                reportError(fmt::format("on line {}:\n{}\nlabel {} missing comma",
                    errorInfo.first,
                    errorInfo.second,
                    errorString));
            }
            token = lex.nextToken();
        }

        if (tokenIsInstruction(token.first)) {
            if (tokenHasZeroOperands(token.first)) {
                instructions.push_back(InstructionData {
                    .instr = tokenToInstruction(token.first),
                    .dataType = DataType::Literal,
                    .textLocation = token.second,
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
                        .textLocation = operands.second,
                        .identifier = lex.getPrevString() });
                    pos++;
                } else if (operands.first == Token::HexNumber || operands.first == Token::DecNumber) {
                    auto prevString = lex.getPrevString();
                    Word value;
                    if (operands.first == Token::HexNumber) {
                        (void)std::from_chars(prevString.data(), prevString.data() + prevString.length(), value, 16);
                    } else {
                        (void)std::from_chars(prevString.data(), prevString.data() + prevString.length(), value, 10);
                    }
                    if (value >= maxAddressSize()) {
                        auto errorInfo = lex.getLine(operands.second);
                        reportError(fmt::format("[parser error] on line {}:\n{}\noperand {} outside of max word range (2^12)",
                            errorInfo.first,
                            errorInfo.second,
                            prevString));
                        value = 0;
                    }
                    instructions.push_back({
                        .instr = tokenToInstruction(token.first),
                        .dataType = DataType::Literal,
                        .textLocation = token.second,
                        .literal = value,
                    });
                    pos++;
                } else {
                    auto errorInfo = lex.getLine(operands.second);
                    reportError(fmt::format("[parser error] on line {}:\n{}\ninvalid operand {}",
                        errorInfo.first,
                        errorInfo.second,
                        lex.getPrevString()));
                }
            }
        } else if (token.first == Token::HexNumber || token.first == Token::DecNumber) {
            auto prevString = lex.getPrevString();
            Word value;
            if (token.first == Token::HexNumber) {
                (void)std::from_chars(prevString.data(), prevString.data() + prevString.length(), value, 16);
            } else {
                (void)std::from_chars(prevString.data(), prevString.data() + prevString.length(), value, 10);
            }
            instructions.push_back({
                .dataType = DataType::Word,
                .textLocation = token.second,
                .literal = value,
            });
            pos++;
        } else {
            auto errorInfo = lex.getLine(token.second);
            reportError(fmt::format("[parser error] on line {}:\n{}\nunexpected token \"{}\"",
                errorInfo.first,
                errorInfo.second,
                tokenToString(token.first)));
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
            if (labels.contains(instr.identifier)) {
                instruction |= (labels[instr.identifier] & 0x0fff);
            } else {
                auto errorInfo = lex.getLine(instr.textLocation);
                reportError(fmt::format("error on line: {}\n{}\nlabel \"{}\" does not exist",
                    errorInfo.first,
                    errorInfo.second,
                    instr.identifier));
            }
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

    if (hasErrors) {
        throw std::runtime_error("parser has errors, cannot output a program");
    }

    return binaryInstructions;
}
