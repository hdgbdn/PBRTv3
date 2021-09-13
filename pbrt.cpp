#include "pbrt.h"
#include "api.h"
#include "parser.h"
#include "scene.h"

using namespace pbrt;

int main(int argc, char* argv[])
{
	Options options;
	std::vector<std::string> fileNames;
	if(fileNames.empty())
	{
		//TODO Parse scene from standard input
	}
	else
	{
		// Parse scene from input files
		for(const auto &f : fileNames)
		{
			pbrtParseFile(f);
		}
	}
	pbrtInit(options);
	//TODO Process scene description
	pbrtCleanUp();
	return 0;
}