#include "assemble.hpp"

#include "file.hpp"
#include "instructions.hpp"
#include "static_hashtable.hpp"

namespace {

std::vector<Word> assembleFromFile(const char* input);
std::vector<Word> assembleFromText(const char* input, std::size_t size);

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
    return (static_cast<int>(tok) >= static_cast<int>(Token::Jns) && static_cast<int>(tok) <= static_cast<int>(Token::StoreI));
}

Instruction tokenToInstruction(Token tok)
{
    if (!tokenIsInstruction(tok)) {
        reportError(fmt::format("token {} is not an instruction", tokenToString(tok)));
    }

    constexpr int offset = static_cast<int>(Token::Jns) - static_cast<int>(Instruction::Jns);

    return static_cast<Instruction>(static_cast<int>(tok) - offset);
}

bool tokenHasZeroOperands(Token tok)
{
    return (static_cast<int>(tok) >= static_cast<int>(Token::Input) && static_cast<int>(tok) <= static_cast<int>(Token::Halt)) || tok == Token::Clear;
}

struct Lexer {
    explicit Lexer(const std::string_view text);

    std::pair<Token, std::size_t> nextToken();

    std::string_view getPrevString();
    std::pair<std::size_t, std::string_view> getLine(std::size_t textLocation);

private:
    std::string_view mText {};
    std::size_t mTextLocation {};
    std::size_t mLineNumber = 1;

    // map of which index a new line is contained
    // text location, line number
    std::map<std::size_t, std::size_t> mNewLines {};

    std::string_view mPrevString {};
    static constexpr std::size_t PrevStringLowerBufferSize = 15;
    std::array<char, PrevStringLowerBufferSize> mPrevStringLower;

    char nextChar();
    char peekChar();
    void consumeChar();

    static constexpr bool isNum(const char c);
    static constexpr bool isAlpha(const char c);
    static constexpr bool isAlphaNum(const char c);
    static constexpr bool isWhiteSpace(const char c);

    // void consumeUntilNewline();
};

constexpr std::size_t string_view_hash(std::string_view str)
{
    std::size_t hash = 0;
    for (auto c : str) {
        hash = static_cast<std::size_t>(c) + (hash << 6) + (hash << 16) - hash;
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
    : mText(text)
{
    // insert the first line into the map
    mNewLines.insert(std::pair(0, 1));
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
        return { Token::Eof, mTextLocation };
    }

    if (c == ';') {
        c = nextChar();
        while (c != '\n' && c != '\0') {
            c = nextChar();
        }
        // need to check for whitespace again
        goto start;
    }

    std::size_t startLocation = mTextLocation - 1;
    if (isAlpha(c)) {
        c = peekChar();
        while (isAlphaNum(c)) {
            consumeChar();
            c = peekChar();
        }
        // fmt::print("{} is not alpha\n", c);

        mPrevString = std::string_view(mText.data() + startLocation, mTextLocation - startLocation);
        // fmt::print("prev string [{}]\n", prevString);

        std::size_t lowerLen = std::min(mPrevString.length(), PrevStringLowerBufferSize);
        for (std::size_t i = 0; i < lowerLen; i++) {
            mPrevStringLower[i] = static_cast<char>(std::tolower(mPrevString[i]));
        }

        auto result = keywords.get(std::string_view { mPrevStringLower.data(), lowerLen });
        // result will be 0 (aka Token::Label) if not found
        return { result, startLocation };
    }

    if (isNum(c)) {
        bool isHex = false;
        if (c == '0') {
            const char newC = peekChar();
            if (static_cast<char>(std::tolower(newC)) == 'x') {
                consumeChar();
                c = nextChar();
                isHex = true;
            }
        }
        startLocation = mTextLocation - 1;

        if (!isNum(c)) {
            // this has to be hex, correct me if I'm wrong
            auto errorInfo = getLine(mTextLocation - 1);
            reportError(fmt::format("on line {}\n{}\nexpected a number after 0x instead got {}",
                errorInfo.first,
                errorInfo.second,
                c));
            mPrevString = "0";
            return { Token::HexNumber, startLocation };
        }

        c = peekChar();
        while (isNum(c)) {
            consumeChar();
            c = peekChar();
        }

        mPrevString = std::string_view { mText.data() + startLocation, mText.data() + mTextLocation };

        if (isHex) {
            return { Token::HexNumber, startLocation };
        } else {
            return { Token::DecNumber, startLocation };
        }
    }

    if (c == ',') {
        return { Token::Comma, startLocation };
    }
    if (c == ':') {
        return { Token::Comma, startLocation };
    }

    auto errorInfo = getLine(mTextLocation);
    reportError(fmt::format("[lexer error] on line {}\n{}\nunexpected character: [{}], [{}]",
        errorInfo.first,
        errorInfo.second,
        c,
        int(c)));

    return { Token::Unknown, startLocation };
}

std::string_view Lexer::getPrevString()
{
    return mPrevString;
}

std::pair<size_t, std::string_view> Lexer::getLine(std::size_t textLocation)
{
    auto itr = mNewLines.upper_bound(textLocation);
    std::size_t start {};
    std::size_t lineNum {};
    itr--;

    start = itr->first;
    lineNum = itr->second;

    while (textLocation < mText.size() && mText[textLocation] != '\n' && mText[textLocation] != '\0') {
        textLocation++;
    }

    if (textLocation < start) {
        throw std::runtime_error("[lexer error] trying to construct a string_view with a start position after the end position, report this bug please.");
    }

    return {
        lineNum,
        std::string_view { mText.data() + start, mText.data() + textLocation }
    };
}

char Lexer::nextChar()
{
    if (mTextLocation >= mText.size()) {
        return '\0';
    }
    char c = *(mText.data() + mTextLocation++);

    if (c == '\n') {
        mLineNumber += 1;
        mNewLines.insert(std::pair(mTextLocation, mLineNumber));
    }

    return c;
}

char Lexer::peekChar()
{
    if (mTextLocation >= mText.size()) {
        return '\0';
    }
    return *(mText.data() + mTextLocation);
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

// void Lexer::consumeUntilNewline()
// {
//     char c = nextChar();
//     while (true) {
//         if (c == '\n' || c == '\0') {
//             break;
//         }
//         c = nextChar();
//     }
// }

std::vector<Word> assembleFromFile(const char* input)
{
    std::vector<char> data = fileToVector<char>(input);
    data.emplace_back('\0');

    return assembleFromText(data.data(), data.size());
}

std::vector<Word> assembleFromText(const char* input, std::size_t size)
{
    Lexer lex({ input, size });

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
                    Word value {};
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
            Word value {};
            if (token.first == Token::HexNumber) {
                (void)std::from_chars(prevString.data(), prevString.data() + prevString.length(), value, 16);
            } else {
                (void)std::from_chars(prevString.data(), prevString.data() + prevString.length(), value, 10);
            }
            instructions.push_back(InstructionData {
                .instr = Instruction::Unknown,
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
    // Vector binaryInstructions {};
    // binaryInstructions.size = instructions.size();
    // binaryInstructions.buffer = (Word*)malloc(instructions.size() * sizeof(Word));

    std::vector<Word> binaryInstructions;
    binaryInstructions.reserve(instructions.size());
    // std::size_t binaryInstructionCount = 0;

    // auto pushBack = [&](Word word) {
    //     binaryInstructions[binaryInstructionCount++] = word;
    // };

    for (auto& instr : instructions) {
        switch (instr.dataType) {
        case DataType::Word: {
            // pushBack(instr.literal);
            binaryInstructions.push_back(instr.literal);
            break;
        }
        case DataType::Identifier: {
            constexpr Word shift = 12U;
            Word instruction = static_cast<Word>(static_cast<Word>(instr.instr) << shift);
            // instruction = instruction << 12;
            if (labels.contains(instr.identifier)) {
                instruction |= (labels[instr.identifier] & 0x0fff);
            } else {
                auto errorInfo = lex.getLine(instr.textLocation);
                reportError(fmt::format("error on line: {}\n{}\nlabel \"{}\" does not exist",
                    errorInfo.first,
                    errorInfo.second,
                    instr.identifier));
            }
            // pushBack(instruction);
            binaryInstructions.push_back(instruction);
            break;
        }
        case DataType::Literal: {
            constexpr Word shift = 12U;
            Word instruction = static_cast<Word>(static_cast<Word>(instr.instr) << shift);
            instruction |= (instr.literal & 0x0fff);
            // pushBack(instruction);
            binaryInstructions.push_back(instruction);
            break;
        }
        default:
            throw std::runtime_error(fmt::format("cannot interpret DataType (eof or unknown) {}", static_cast<int>(instr.dataType)));
        }
    }

    if (hasErrors) {
        throw std::runtime_error("parser has errors, cannot output a program");
    }

    return binaryInstructions;
}

} // anonymous namespace

int assemble(const char* input, const char* output)
{
    try {
        std::vector<Word> values = assembleFromFile(input);

        for (auto& value : values) {
            value = std::rotr(value, 8);
            LOGT("value: {:x}", value);
        }

        dataToFile(output, std::span(values));

        return 0;
    } catch (std::runtime_error& error) {
        fmt::print("{}\n", error.what());
        return 1;
    }
}

int assembleToVec(const char* input, const char* outputFile, std::vector<Word>* output)
{
    try {
        if (outputFile != nullptr) {
            std::vector<Word> values = assembleFromFile(input);

            for (auto& value : values) {
                value = std::rotr(value, 8);
            }

            dataToFile(outputFile, std::span(values));

            *output = std::move(values);
        } else {
            *output = assembleFromFile(input);
        }
        return 0;
    } catch (std::runtime_error& error) {
        fmt::print("{}\n", error.what());
        return 1;
    }
}
