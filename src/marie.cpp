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
    std::tuple<Instruction, word> decode(word instr);
    void execInstr(std::tuple<Instruction, word>& instr);

private:
    static constexpr size_t MaxMemory = 4096 * sizeof(byte);
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
    return 0;
}

std::tuple<Instruction, word> Marie::decode(word instr)
{
    std::tuple<Instruction, word> val;
    std::get<0>(val) = (Instruction)(instr >> 12 & 0xF);
    std::get<1>(val) = (word)(instr & 0xFFF0);
    return val;
}

void Marie::execInstr(std::tuple<Instruction, word>& instr)
{
    if (skipNext) {
        skipNext = false;
        return;
    }

    switch (std::get<0>(instr)) {
    case Instruction::Load:
		AC = memoryAtAddress(std::get<1>(instr));
		break;
    case Instruction::Store:
		storeAtAddress(std::get<1>(instr));
		break;
    case Instruction::Add:
		AC = AC + memoryAtAddress(std::get<1>(instr));
		break;
    case Instruction::Subt:
		AC = AC - memoryAtAddress(std::get<1>(instr));
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
        skipNext = true;
		break;
    case Instruction::Jump:
		PC = std::get<1>(instr);
		break;
    default:
        fmt::print("Invalid instruction {}", (int)std::get<0>(instr));
    }
}

word Marie::memoryAtAddress(word address)
{
    if (address < MaxMemory) {
        fmt::print("attempting to address outside of memory, returning 0\n");
        return 0;
    }
    return *(memory + address);
}

void Marie::storeAtAddress(word address)
{
    if (address < MaxMemory) {
        fmt::print("attempting to address outside of memory, doing nothing\n");
        return;
    }
    *(memory + address) = AC;
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
