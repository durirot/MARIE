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
};
