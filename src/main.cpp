#include "assemble.hpp"
#include "disassemble.hpp"
#include "marie.hpp"

enum Operation {
    None,
    Execfile,
    Execbin,
    Assemble,
    Disassemble,
};

struct ArgParser {
    explicit ArgParser(const std::span<char*>& args);
    int invalidArgs();

    bool invalid = false;
    char* input = nullptr;
    char* output = nullptr;
    Operation operation = None;

private:
    std::span<char*> args;
};

ArgParser::ArgParser(const std::span<char*>& args)
    : args(args)
{
    for (std::size_t i = 1; i < args.size(); i++) {
        if (strcmp(args[i], "-o") == 0) {
            if (i + 1 < args.size()) {
                i++;
                output = args[i];
            } else {
                fmt::print("no output file given after \"-o\"\n");
                invalid = true;
            }
        } else if (strcmp(args[i], "exec-bin") == 0) {
            operation = Execbin;
        } else if (strcmp(args[i], "exec-file") == 0) {
            operation = Execfile;
        } else if (strcmp(args[i], "assemble") == 0) {
            operation = Assemble;
        } else if (strcmp(args[i], "disassemble") == 0) {
            operation = Disassemble;
        } else {
            input = args[i];
        }
    }
}

int ArgParser::invalidArgs()
{
    fmt::print("Usage {} [command] [input] -o [output]\nCommands: assemble, exec-file, exec-bin, disassemble\n", args[0]);
    return -1;
}

int main(int argc, char** argv)
{
    ArgParser parser(std::span(argv, static_cast<std::size_t>(argc)));

    if (parser.invalid) {
        return parser.invalidArgs();
    } else {
        switch (parser.operation) {
        case Assemble: {
            if (parser.input == nullptr) {
                fmt::print("No inputs given\n");
                return parser.invalidArgs();
            }
            if (parser.output == nullptr) {
                fmt::print("No outputs given\n");
                return parser.invalidArgs();
            }
            return assemble(parser.input, parser.output);
        } // Assemble
        case Execfile: {
            if (parser.input == nullptr) {
                fmt::print("No inputs given\n");
                return parser.invalidArgs();
            }
            std::vector<Word> program {};
            if (assembleToVec(parser.input, parser.output, &program) != 0) {
                return 1;
            }
            return marieExecuteVec(program);
        } // Exec
        case Execbin: {
            if (parser.input == nullptr) {
                fmt::print("No inputs given\n");
                return parser.invalidArgs();
            }
            return marieExecute(parser.input);
        } // Execbin
        case Disassemble: {
            if (parser.input == nullptr) {
                fmt::print("No inputs given\n");
                return parser.invalidArgs();
            }
            if (parser.output == nullptr) {
                return disassembleAndPrint(parser.input);
            }
            return disassembleToFile(parser.input, parser.output);
        } // Disassemble
        default:
            fmt::print("No operation given\n");
            return parser.invalidArgs();
        }
    }

    return 0;
}
