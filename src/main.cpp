#include "marie.h"
#include "assemble.hpp"

#include <fmt/core.h>
#include <fstream>

void assemble(const char* input, const char* output) 
{
    try {
        std::vector<Word> values = assembleFromFile(input);
        std::ofstream file;
        file.open(output);
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

Word execute(const char* input)
{
	return marieLoad(input);
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        fmt::print("Expected at least 3 arguments:\nUsage {} <file> <output>\n", argv[0]);
        return -1;
    }

	assemble(argv[1], argv[2]);
	// execute(argv[2]);

}
