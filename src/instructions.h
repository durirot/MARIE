#pragma once

enum struct Instruction {
    Jns = 0,
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
    StoreI,
    LoadI,
    Unknown,
};

constexpr const char* InstructionToString(Instruction instr)
{
    switch (instr) {
    case Instruction::Jns:
        return "Jns";
    case Instruction::Load:
        return "Load";
    case Instruction::Store:
        return "Store";
    case Instruction::Add:
        return "Add";
    case Instruction::Subt:
        return "Subt";
    case Instruction::Input:
        return "Input";
    case Instruction::Output:
        return "Output";
    case Instruction::Halt:
        return "Halt";
    case Instruction::Skipcond:
        return "Skipcond";
    case Instruction::Jump:
        return "Jump";
    case Instruction::Clear:
        return "Clear";
    case Instruction::AddI:
        return "AddI";
    case Instruction::JumpI:
        return "JumpI";
    case Instruction::StoreI:
        return "StoreI";
    case Instruction::LoadI:
        return "LoadI";
    case Instruction::Unknown:
        return "Unknown";
	default:
		return "Invalid instruction";
    }
}
