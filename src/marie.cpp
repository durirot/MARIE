#include "marie.h"

#include <cstdio>
#include <fmt/core.h>
#include <tuple>

#include "instructions.h"

struct Marie {
    Marie(Byte* image, size_t imageSize);

    Word run();
    std::pair<Instruction, Word> decode(Word instr);
    void execInstr(std::pair<Instruction, Word>& instr);

private:
    static constexpr size_t MaxMemory = 4096 * sizeof(Word);
    Byte memory[MaxMemory] {};

    Word AC {}; // Accumulator
    // Word MAR {}; // Memory Address Register
    // Word MBR {}; // Memory Buffer Register
    Word PC {}; // Program Counter
    // Word IR {}; // Instruction Register (holds the next expression to be executed)
    // Word InREG {}; //  Input Register (holds data from the input device)

    bool skipNext = false;
    bool errors = false;
    bool halt = false;

    Word memoryAtAddress(Word address);
    void storeAtAddress(Word address);
    bool skipCond(Word condition);
};

Marie::Marie(Byte* image, size_t imageSize)
{
    if (imageSize > MaxMemory) {
        fmt::print("Warning, an image size of {} is larger than MARIE's max memory of {} Bytes\n", imageSize, MaxMemory);
        imageSize = MaxMemory;
    }
    memcpy(memory, image, imageSize * sizeof(Byte));
}

Word Marie::run()
{
    PC = memoryAtAddress(0);

    while (halt != true) {
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
    val.second = (Word)(instr & 0xFFF0);
    return val;
}

void Marie::execInstr(std::pair<Instruction, Word>& instr)
{
    if (skipNext) {
        skipNext = false;
        return;
    }

    switch (instr.first) {
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
        AC = getchar();
        break;
    case Instruction::Output:
        fmt::print("{}", AC);
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
    default:
        fmt::print("Invalid instruction {}", (int)instr.first);
    }
}

Word Marie::memoryAtAddress(Word address)
{
    if (address < MaxMemory) {
        fmt::print("attempting to address outside of memory, returning 0\n");
        return 0;
    }
    return *(memory + (address * sizeof(Word)));
}

void Marie::storeAtAddress(Word address)
{
    if (address < MaxMemory) {
        fmt::print("attempting to address outside of memory, doing nothing\n");
        return;
    }
    *(memory + (address * sizeof(Word))) = AC;
}

bool Marie::skipCond(Word condition)
{
    condition = condition & 0x0C00;

    if (condition == 0x0000 && AC < 0) {
        return true;
    } else if (condition == 0x0400 && AC == 0) {
        return true;
    } else if (condition == 0x0800 && AC > 0) {
        return true;
    }

    return false;
}

Word marieLoad(const char* file)
{
    FILE* handle = fopen(file, "rb");
    if (handle == nullptr) {
        fmt::print("Could not open file {}\n", file);
        return -1;
    }
    fseek(handle, 0, SEEK_END);
    size_t size = ftell(handle);
    fseek(handle, 0, SEEK_SET);

    Byte* data = (Byte*)malloc(size * sizeof(Byte));
    if (data == nullptr) {
        fmt::print("Failed to allocate memory for file {}\n", file);
        return -2;
    }

    (void)fread(data, size, 1, handle);
    (void)fclose(handle);

    Marie vm(data, size);

    Word result = vm.run();

    free(data);
    return result;
}
