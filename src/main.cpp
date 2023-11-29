// #include "marie.h"
#include "assemble.h"

#include <fmt/core.h>
#include <fstream>

int main(int argc, char** argv)
{
    if (argc < 3) {
        fmt::print("Expected at least 3 arguments:\nUsage {} <file> <output>\n", argv[0]);
        return -1;
    }

    // const char* text("test, load 0x5\nstore test");

    try {
        std::vector<Word> values = assembleFromFile(argv[1]);
        std::ofstream file;
        file.open(argv[2]);
        for (Word value : values) {
            file.write(((char*)&value) + 1, 1);
            file.write((char*)&value, 1);
            // fmt::print("{:x}\n", value);
        }
        file.close();
    } catch (std::runtime_error error) {
        fmt::print("{}\n", error.what());
    }
}
