# MARIE 

A MARIE virtual machine

has various presets that can be used with cmake --preset=config
example:
 
cmake --preset=linux-clang-debug &&
ninja -C build/debug

# package manager
using cpm as a package manager

# clang format/tidy
copied the clang format and tidy files from lefticus's cmake preset github page, made some slight modifications like tab sizes
