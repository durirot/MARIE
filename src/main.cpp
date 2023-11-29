// #include "marie.h"
#include "assemble.h"

#include <fmt/core.h>

int main(int argc, char** argv)
{
    // if (argc < 2) {
    // 	fmt::print("Expected at least two arguments:\nUsage marievm <file>\n");
    // 	return -1;
    // }

    try {
        std::vector<Word> values = assembleFromText(R"(
			test, load 5
			store test
		)");
        for (Word value : values) {
            fmt::print("#x{:x}\n", value);
        }
    } catch (std::runtime_error error) {
        fmt::print("{}\n", error.what());
    }

    // return marieLoad(argv[2]);
}
