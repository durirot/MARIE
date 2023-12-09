# MARIE 

A MARIE virtual machine

# Usage

marievm [command] [input] -o [output]
- assemble  (assembles to an output file in big endian)
- exec-bin  (execs a big endian binary file)
- exec-file (execs a file that has not been assembled yet)

# Building

Can be built with various presets that can be used with cmake --preset=config
for example:
 
cmake --preset=linux-clang-debug &&
ninja -C build/debug
