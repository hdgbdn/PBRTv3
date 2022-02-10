#include "core/pbrt.h"
#include "core/api.h"

using namespace pbrt;

int main(int argc, char* argv[])
{
	Options options;
	std::vector<std::string> fileNames;
	// Process command-line arguments
	for(int i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "--nthreads") || !strcmp(argv[i], "-nthreads")) {
			if (i + 1 == argc)
				//usage("missing value after --nthreads argument");
			options.nThreads = atoi(argv[++i]);
		}
		else
			fileNames.push_back(argv[i]);
	}
	// Print welcome banner
	if (sizeof(void*) == 4)
		fmt::print("*** WARNING: This is a 32-bit build of pbrt. It will crash "
			"if used to render highly complex scenes. ***\n");
#ifndef NDEBUG
	fmt::print("*** DEBUG BUILD ***\n");
#endif // !NDEBUG
	fmt::print("2021-2022 Hdgbdn.\n");
	fmt::print("The source code to pbrt is covered by the BSD License.\n");
		
	pbrtInit(options);
	if(fileNames.empty())
	{
		pbrtParseFile("-");
	}
	else
	{
		// Parse scene from input files
		for(const auto &f : fileNames)
		{
			pbrtParseFile(f);
		}
	}
	pbrtCleanUp();
	return 0;
}