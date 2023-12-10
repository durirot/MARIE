#include "disassemble.h"

#include "common.h"
#include "instructions.h"
#include <cstdio>
#include <fmt/core.h>

namespace {

Vector readFile(const char* inputFile)
{
    FILE* file = fopen(inputFile, "rb");
    if (file == nullptr) {
        throw std::runtime_error(fmt::format("cannot open input file, {}", inputFile));
    }

    fseek(file, 0, SEEK_END);
    std::size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    Word* data = (Word*)malloc(size * sizeof(char));
    if (data == nullptr) {
        throw std::runtime_error("failed to allocate memory??? how!");
    }

    (void)fread(data, 1, size, file);

    fclose(file);

    return Vector { size / 2, data };
}

std::pair<Instruction, Word> decodeInstruction(Word instr)
{
    std::pair<Instruction, Word> val;
    val.first = (Instruction)(instr >> 12 & 0xF);
    val.second = (Word)(instr & 0x0FFF);
    return val;
}

bool instrHasZeroOperands(Instruction tok)
{
    return ((int)tok >= (int)Instruction::Input && (int)tok <= (int)Instruction::Halt) || tok == Instruction::Clear;
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
    Vector data = readFile(inputFile);

    for (std::size_t i = 0; i < data.size; i++) {
        appendInstruction(std::rotr(data.buffer[i], 8), output);
    }

    free(data.buffer);

    return output;
}

void writeToFile(std::string& data, const char* outputFile)
{
    FILE* file = fopen(outputFile, "w");
    if (file == nullptr) {
        throw std::runtime_error(fmt::format("cannot open output file, {}", outputFile));
    }

    (void)fwrite(data.data(), 1, data.size(), file);

    (void)fclose(file);
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
        writeToFile(data, output);
        return 0;
    } catch (std::runtime_error& error) {
        fmt::print("{}", error.what());
        return 1;
    }
}
