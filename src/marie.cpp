#include "marie.h"

#include "instructions.h"

#include <charconv>
#include <cstdio>
#include <fmt/core.h>
#include <iostream>

namespace {

enum Level {
    None,
    Debug,
    Trace,
};
constexpr Level LogLevel = Level::Debug;

#define Log(requiredLevel, ...)                          \
    if constexpr ((int)LogLevel >= (int)requiredLevel) { \
        fmt::print(__VA_ARGS__);                         \
    }

} // anonymous namespace

struct Marie {
    Marie(Word* image, size_t imageSize);

    Word run();
    std::pair<Instruction, Word> decode(Word instr);
    void execInstr(std::pair<Instruction, Word>& instr);

private:
    static constexpr std::size_t MaxMemory = 4096;
    Word memory[MaxMemory] {};
	std::size_t imageSize {};

    Word AC {}; // Accumulator
    // Word MAR {}; // Memory Address Register
    // Word MBR {}; // Memory Buffer Register
    Word PC {}; // Program Counter
    // Word IR {}; // Instruction Register (holds the next expression to be executed)
    // Word InREG {}; //  Input Register (holds data from the input device)

    bool skipNext = false;
    // bool errors = false;
    bool halt = false;

    Word userInputHex();
    Word memoryAtAddress(Word address);
    void storeAtAddress(Word address);
    bool skipCond(Word condition);
};

Marie::Marie(Word* image, size_t imageSize)
	: imageSize(imageSize)
{
    if (imageSize > MaxMemory) {
        fmt::print("Warning, an image size of {} is larger than MARIE's max memory of {} Bytes\n", imageSize, MaxMemory);
        imageSize = MaxMemory;
    }
    memcpy(memory, image, imageSize * sizeof(Word));
}

Word Marie::run()
{
    PC = 0;

    while (halt != true && PC < imageSize) {
        auto instr = decode(memoryAtAddress(PC));
        PC += 1;
        execInstr(instr);
    }

    return AC;
}

std::pair<Instruction, Word> Marie::decode(Word instr)
{
    std::pair<Instruction, Word> val;
    val.first = (Instruction)(instr >> 12 & 0xF);
    val.second = (Word)(instr & 0x0FFF);
    return val;
}

void Marie::execInstr(std::pair<Instruction, Word>& instr)
{
    if (skipNext) {
        Log(Debug, "skipping instruction\n");
        skipNext = false;
        return;
    }

    Log(Debug, "executing instruction {}\n", InstructionToString(instr.first));
    switch (instr.first) {
    case Instruction::Jns: {
        AC = PC;
        storeAtAddress(instr.second);
        AC = instr.second + 1;
        PC = AC;
        break;
    }
    case Instruction::Load:
        AC = memoryAtAddress(instr.second);
        break;
    case Instruction::Store:
        storeAtAddress(instr.second);
        break;
    case Instruction::Add:
        AC = AC + memoryAtAddress(instr.second);
        break;
    case Instruction::Subt:
        AC = AC - memoryAtAddress(instr.second);
        break;
    case Instruction::Input:
        AC = userInputHex();
        break;
    case Instruction::Output:
        fmt::print("{:x}\n", AC);
        break;
    case Instruction::Halt:
        halt = true;
        break;
    case Instruction::Skipcond:
        skipNext = skipCond(instr.second);
        break;
    case Instruction::Jump:
        PC = instr.second;
        break;
    case Instruction::Clear:
        AC = 0;
        break;
    case Instruction::AddI:
        AC = AC + memoryAtAddress(memoryAtAddress(instr.second));
        break;
    case Instruction::JumpI:
        PC = memoryAtAddress(instr.second) & 0x0FFF;
        break;
    case Instruction::LoadI:
        AC = memoryAtAddress(memoryAtAddress(instr.second));
        break;
    case Instruction::StoreI:
        storeAtAddress(memoryAtAddress(instr.second));
        break;
    default:
        fmt::print("Invalid instruction {:x} at PC {:x}\n", (int)instr.first, PC);
    }
}

Word Marie::userInputHex()
{
    std::string line;
    std::getline(std::cin, line);

    Word value;
    std::from_chars(line.data(), line.data() + line.length(), value, 16);

    return value;
}

Word Marie::memoryAtAddress(Word address)
{
    if (address >= imageSize) {
        fmt::print("attempting to address outside of memory at {:x}, returning 0 and halting\n", address);
		halt = true;
        return 0;
    }
    return *(memory + address);
}

void Marie::storeAtAddress(Word address)
{
    if (address >= imageSize) {
        fmt::print("attempting to address outside of memory, doing nothing and halting\n");
		halt = true;
        return;
    }
    *(memory + address) = AC;
}

bool Marie::skipCond(Word condition)
{
    condition = condition & 0x0C00;

    if (condition == 0x0000 && (int16_t)AC < 0) {
        return true;
    } else if (condition == 0x0400 && (int16_t)AC == 0) {
        return true;
    } else if (condition == 0x0800 && (int16_t)AC > 0) {
        return true;
    }

    return false;
}

Word marieExecute(const char* file)
{
    FILE* handle = fopen(file, "rb");
    if (handle == nullptr) {
        fmt::print("Could not open file {}\n", file);
        return -1;
    }
    fseek(handle, 0, SEEK_END);
    std::size_t size = ftell(handle) / (sizeof(Word)/sizeof(Byte));
    fseek(handle, 0, SEEK_SET);

    Word* data = (Word*)malloc(size * sizeof(Word));
    if (data == nullptr) {
        fmt::print("Failed to allocate memory for file {}\n", file);
        return -2;
    }

    (void)fread(data, size, 2, handle);
    (void)fclose(handle);

    for (std::size_t i = 0; i < size; i++) {
        *(data + i) = std::rotr(*(data + i), 8);
    }

    Marie vm(data, size);
    Word result = vm.run();

    free(data);
    return result;
}

Word marieExecuteVec(Vector* program)
{
    Marie vm(program->buffer, program->size);
    Word result = vm.run();

    free(program->buffer);
    return result;
}
