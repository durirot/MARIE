#include "marie.h"

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

	std::tuple<Instruction, word> decode();

	byte memory[4096];
	
	word MAR;
	word MBR;
	word AC;
};

Marie::Marie(byte* image)
{
	
}

std::tuple<Instruction, word> Marie::decode()
{

}

word marieLoad(const char* file)
{
	return 0;
}
