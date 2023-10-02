#include "marie.h"

#include <fmt/core.h>

int main(int argc, char** argv)
{
	if (argc < 2) {
		fmt::print("Expected at least two arguments:\nUsage marievm <file>\n");
		return -1;
	}

	return marieLoad(argv[2]);
}
