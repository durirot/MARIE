#include "marie.h"

#include <cstdio>
#include <fmt/core.h>
#include <tuple>

enum struct Instruction {
    Load,
    Store,
    Add,
    Sub,
};

struct Marie {
    Marie(byte* image);

    word run();
    std::tuple<Instruction, word> decode();

private:
    byte memory[4096];

    byte* image;

    word AC; // Accumulator
    word MAR; // Memory Address Register
    word MBR; // Memory Buffer Register
    word PC; // Program Counter
    word IR; // Instruction Register (holds the next expression to be executed)
    word InREG; //  Input Register (holds data from the input device)
};

Marie::Marie(byte* image)
    : image(image)
{
}

word Marie::run()
{
    return 0;
}

std::tuple<Instruction, word> Marie::decode()
{
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

    Marie vm(data);

    word result = vm.run();

    free(data);
    return result;
}
