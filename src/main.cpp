#include "assemble.h"
#include "marie.h"

#include <fmt/core.h>

int main(int argc, char** argv)
{
    if (argc < 3) {
        fmt::print("Expected at least 3 arguments:\nUsage {} <file> <output>\n", argv[0]);
        return -1;
    }

    if (assemble(argv[1], argv[2]) != 0) {
        return -2;
    }

    return marieExecute(argv[2]);
}
