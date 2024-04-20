#include "marie.h"

#include "file.hpp"
#include "instructions.hpp"

namespace {

struct Marie {
    Marie(const Word* image, size_t imageSize);

    Word run();
    static std::pair<Instruction, Word> decode(Word instr);
    void execInstr(std::pair<Instruction, Word>& instr);

private:
    static constexpr std::size_t MaxMemory = 4096;
    std::array<Word, MaxMemory> mMemory {};
    std::size_t mImageSize {};

    Word mAC {}; // Accumulator
    // Word MAR {}; // Memory Address Register
    // Word MBR {}; // Memory Buffer Register
    Word mPC {}; // Program Counter
    // Word IR {}; // Instruction Register (holds the next expression to be executed)
    // Word InREG {}; //  Input Register (holds data from the input device)

    bool mSkipNext = false;
    // bool errors = false;
    bool mHalt = false;

    [[nodiscard]] static Word userInputHex();
    [[nodiscard]] Word memoryAtAddress(const Word address);
    void storeAtAddress(const Word address);
    [[nodiscard]] bool skipCond(Word condition) const;
};

Marie::Marie(const Word* image, size_t imageSize)
    : mImageSize(imageSize)
{
    if (imageSize > MaxMemory) {
        LOGW("Warning, an image size of {} is larger than MARIE's max memory of {} Bytes\n", imageSize, MaxMemory);
        mImageSize = MaxMemory;
    }
    std::memcpy(mMemory.data(), image, mImageSize * sizeof(Word));
}

Word Marie::run()
{
    mPC = 0;

    while (mHalt && mPC < mImageSize) {
        auto instr = decode(memoryAtAddress(mPC));
        mPC += 1;
        execInstr(instr);
    }

    return mAC;
}

std::pair<Instruction, Word> Marie::decode(Word instr)
{
    std::pair<Instruction, Word> val;
    val.first = static_cast<Instruction>(instr >> 12 & 0xF);
    val.second = static_cast<Word>(instr & 0x0FFF);
    return val;
}

void Marie::execInstr(std::pair<Instruction, Word>& instr)
{
    if (mSkipNext) {
        LOGD("skipping instruction");
        mSkipNext = false;
        return;
    }

    LOGD("executing instruction {}", InstructionToString(instr.first));

    switch (instr.first) {
    case Instruction::Jns: {
        mAC = mPC;
        storeAtAddress(instr.second);
        mAC = instr.second + 1;
        mPC = mAC;
        break;
    }
    case Instruction::Load:
        mAC = memoryAtAddress(instr.second);
        break;
    case Instruction::Store:
        storeAtAddress(instr.second);
        break;
    case Instruction::Add:
        mAC = mAC + memoryAtAddress(instr.second);
        break;
    case Instruction::Subt:
        mAC = mAC - memoryAtAddress(instr.second);
        break;
    case Instruction::Input:
        mAC = userInputHex();
        break;
    case Instruction::Output:
        fmt::print("{:x}\n", mAC);
        break;
    case Instruction::Halt:
        mHalt = true;
        break;
    case Instruction::Skipcond:
        mSkipNext = skipCond(instr.second);
        break;
    case Instruction::Jump:
        mPC = instr.second;
        break;
    case Instruction::Clear:
        mAC = 0;
        break;
    case Instruction::AddI:
        mAC = mAC + memoryAtAddress(memoryAtAddress(instr.second));
        break;
    case Instruction::JumpI:
        mPC = memoryAtAddress(instr.second) & 0x0FFF;
        break;
    case Instruction::LoadI:
        mAC = memoryAtAddress(memoryAtAddress(instr.second));
        break;
    case Instruction::StoreI:
        storeAtAddress(memoryAtAddress(instr.second));
        break;
    default:
        fmt::print("Invalid instruction {:x} at PC {:x}\n", static_cast<int>(instr.first), mPC);
    }
}

[[nodiscard]] Word Marie::userInputHex()
{
    std::string line;
    std::getline(std::cin, line);

    Word value {};
    std::from_chars(line.data(), line.data() + line.length(), value, 16);

    return value;
}

[[nodiscard]] Word Marie::memoryAtAddress(const Word address)
{
    if (address >= mImageSize) {
        fmt::print("attempting to address outside of memory at {:x}, returning 0 and halting\n", address);
        mHalt = true;
        return 0;
    }
    return *(mMemory.data() + address);
}

void Marie::storeAtAddress(const Word address)
{
    if (address >= mImageSize) {
        fmt::print("attempting to address outside of memory, doing nothing and halting\n");
        mHalt = true;
        return;
    }
    *(mMemory.data() + address) = mAC;
}

[[nodiscard]] bool Marie::skipCond(Word condition) const
{
    condition = condition & 0x0C00;

    constexpr Word SkipLt = 0x0000;
    constexpr Word SkipEq = 0x0400;
    constexpr Word SkipGt = 0x0800;

    switch (condition) {
    case SkipLt: {
        if (static_cast<int16_t>(mAC) < 0) {
            return true;
        }
        break;
    }
    case SkipEq: {
        if (static_cast<int16_t>(mAC) == 0) {
            return true;
        }
        break;
    }
    case SkipGt: {
        if (static_cast<int16_t>(mAC) > 0) {
            return true;
        }
        break;
    }
    default:
        break;
    }

    return false;
}

} // anonymous namespace

Word marieExecute(const char* inputFile)
{
    std::vector<Word> data = fileToVector<Word>(inputFile);

    // Convert from big to little endian
    for (Word& i : data) {
        i = std::rotr(i, 8);
        // *(data.data() + i) = std::rotr(*(data.data() + i), 8);
    }

    Marie vm(data.data(), data.size());
    Word result = vm.run();

    return result;
}

Word marieExecuteVec(const std::vector<Word>& program)
{
    Marie vm(program.data(), program.size());
    Word result = vm.run();

    return result;
}
