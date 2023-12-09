#include "assemble.h"
#include "marie.h"

#include <fmt/core.h>
#include <string.h>

enum Operation {
    None,
    Execfile,
    Execbin,
    Assemble,
};

int main(int argc, char** argv)
{
    bool invalid = false;
    char* input = nullptr;
    char* output = nullptr;
    Operation operation = None;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                i++;
                output = argv[i];
            } else {
				fmt::print("no output file given after \"-o\"\n");
                invalid = true;
            }
        } else if (strcmp(argv[i], "exec-bin") == 0) {
            operation = Execbin;
        } else if (strcmp(argv[i], "exec-file") == 0) {
            operation = Execfile;
        } else if (strcmp(argv[i], "assemble") == 0) {
            operation = Assemble;
        } else {
            input = argv[i];
        }
    }

    if (invalid) {
    invalid:
        fmt::print("Usage {} [command] [input] -o [output]\nCommands: assemble, exec-file, exec-bin\n", argv[0]);
        return -1;
    } else {
        switch (operation) {
        case Assemble: {
            if (input == nullptr) {
                fmt::print("No inputs given\n");
                goto invalid;
            }
            if (output == nullptr) {
                fmt::print("No outputs given\n");
                goto invalid;
            }
            return assemble(input, output);
        } // Assemble
        case Execfile: {
            if (input == nullptr) {
                fmt::print("No inputs given\n");
                goto invalid;
            }
            Vector program;
            if (assembleToVec(input, output, &program) != 0) {
                return 1;
            }
            return marieExecuteVec(&program);
        } // Exec
        case Execbin: {
            return marieExecute(input);
        } // Execbin
        default:
            fmt::print("No operation given\n");
            goto invalid;
        }
    }

    return 0;
}
