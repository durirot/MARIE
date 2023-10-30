#include "marie.h"

#include <cstdio>
#include <fmt/core.h>
#include <tuple>

enum struct Instruction {
    Load = 1,
    Store,
    Add,
    Subt,

    Input,
    Output,
    Halt,
    Skipcond,
    Jump,
};

struct Marie {
    Marie(byte* image, size_t imageSize);

    word run();
    std::pair<Instruction, word> decode(word instr);
    void execInstr(std::pair<Instruction, word>& instr);

private:
    static constexpr size_t MaxMemory = 4096 * sizeof(word);
    byte memory[MaxMemory] {};

    word AC {}; // Accumulator
    word MAR {}; // Memory Address Register
    word MBR {}; // Memory Buffer Register
    word PC {}; // Program Counter
    word IR {}; // Instruction Register (holds the next expression to be executed)
    word InREG {}; //  Input Register (holds data from the input device)

    bool skipNext = false;
    bool errors = false;
    bool halt = false;

    word memoryAtAddress(word address);
    void storeAtAddress(word address);
    bool skipCond(word condition);
};

Marie::Marie(byte* image, size_t imageSize)
{
    if (imageSize > MaxMemory) {
        fmt::print("Warning, an image size of {} is larger than MARIE's max memory of {} bytes\n", imageSize, MaxMemory);
        imageSize = MaxMemory;
    }
    memcpy(memory, image, imageSize * sizeof(byte));
}

word Marie::run()
{
    PC = memoryAtAddress(0);

    while (halt != true) {
        auto instr = decode(memoryAtAddress(PC));
        PC += 1;
        execInstr(instr);
    }

    return AC;
}

std::pair<Instruction, word> Marie::decode(word instr)
{
    std::pair<Instruction, word> val;
    val.first = (Instruction)(instr >> 12 & 0xF);
    val.second = (word)(instr & 0xFFF0);
    return val;
}

void Marie::execInstr(std::pair<Instruction, word>& instr)
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

word Marie::memoryAtAddress(word address)
{
    if (address < MaxMemory) {
        fmt::print("attempting to address outside of memory, returning 0\n");
        return 0;
    }
    return *(memory + (address * sizeof(word)));
}

void Marie::storeAtAddress(word address)
{
    if (address < MaxMemory) {
        fmt::print("attempting to address outside of memory, doing nothing\n");
        return;
    }
    *(memory + (address * sizeof(word))) = AC;
}

bool Marie::skipCond(word condition)
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

word marieLoad(const char* file)
{
    FILE* handle = fopen(file, "rb");
    if (handle == nullptr) {
        fmt::print("Could not open file {}\n", file);
        return -1;
    }
    fseek(handle, 0, SEEK_END);
    size_t size = ftell(handle);
    fseek(handle, 0, SEEK_SET);

    byte* data = (byte*)malloc(size * sizeof(byte));
    if (data == nullptr) {
        fmt::print("Failed to allocate memory for file {}\n", file);
        return -2;
    }

    (void)fread(data, size, 1, handle);
    fclose(handle);

    Marie vm(data, size);

    word result = vm.run();

    free(data);
    return result;
}
