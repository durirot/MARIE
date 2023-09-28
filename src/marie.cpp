#include "marie.h"

#include <fmt/core.h>
#include <tuple>
#include <cstdio>

enum struct Instruction {
	Load,
	Store,
	Add,
	Sub,
};

struct Marie {
	Marie(byte* image);

	word run();
	std::tuple<Instruction, word> decode();


private:
	byte memory[4096];
	
	word MAR;
	word MBR;
	word AC;
};

Marie::Marie(byte* image)
{
	
}

word Marie::run()
{
	return 0;
}

std::tuple<Instruction, word> Marie::decode()
{

}

word marieLoad(const char* file)
{
	FILE* handle = fopen(file, "rb");
	if (handle == nullptr) {
		fmt::print("Could not open file {}\n", file);
		return -1;
	}
	fseek(handle, 0, SEEK_END);
	size_t size = ftell(handle);
	fseek(handle, 0, SEEK_SET);

	byte* data = (byte*)malloc(size*sizeof(byte));
	if (data == nullptr) {
		fmt::print("Failed to allocate memory for file {}\n", file);
		return -2;
	}

	(void)fread(data, size, 1, handle);
	fclose(handle);

	Marie vm(data);

	word result = vm.run();

	free(data);
	return result;
}
