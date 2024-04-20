#include "disassemble.h"

#include "file.hpp"
#include "instructions.hpp"

namespace {

std::pair<Instruction, Word> decodeInstruction(Word instr)
{
    std::pair<Instruction, Word> val;
    val.first = static_cast<Instruction>(instr >> 12 & 0xF);
    val.second = static_cast<Word>(instr & 0x0FFF);
    return val;
}

bool instrHasZeroOperands(Instruction tok)
{
    return (static_cast<int>(tok) >= static_cast<int>(Instruction::Input) && static_cast<int>(tok) <= static_cast<int>(Instruction::Halt)) || tok == Instruction::Clear;
}

void appendInstruction(Word instruction, std::string& output)
{
    auto instr = decodeInstruction(instruction);
    if (instrHasZeroOperands(instr.first)) {
        output += fmt::format("{}\n", InstructionToString(instr.first));
    } else {
        output += fmt::format("{} {:x}\n", InstructionToString(instr.first), instr.second);
    }
}

std::string disassembleToString(const char* inputFile)
{
    std::string output {};
    std::vector<Word> data = fileToVector<Word>(inputFile);

    for (Word instr : data) {
        appendInstruction(std::rotr(instr, 8), output);
    }

    return output;
}

} // anonymous namespace

int disassembleAndPrint(const char* input)
{
    try {
        fmt::print("{}", disassembleToString(input));
        return 0;
    } catch (std::runtime_error& error) {
        fmt::print("{}", error.what());
        return 1;
    }
}

int disassembleToFile(const char* input, const char* output)
{
    try {
        std::string data = disassembleToString(input);
        dataToFile(output, std::span(data));
        return 0;
    } catch (const std::runtime_error& error) {
        fmt::print("{}", error.what());
        return 1;
    }
}
