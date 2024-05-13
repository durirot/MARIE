# MARIE 

A MARIE virtual machine, assembler and disassembler

# Usage

marievm [command] [input] -o [output]
- assemble    (assembles to an output file in big endian)
- exec-bin    (execs a big endian binary file)
- exec-file   (execs a file that has not been assembled yet)
- disassemble (disassembles to standard out, or to a specified output file)

# Building

Can be built with various presets that can be used with cmake --preset=config
for example:
 
cmake --preset=linux-clang-debug &&
ninja -C build/debug
