#pragma once

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
