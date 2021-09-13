#ifndef PBRT_CORE_API_H
#define PBRT_CORE_API_H

#include "pbrt.h"

namespace pbrt
{
	void pbrtInit(const Options& opt);
	void pbrtCleanUp();
	void pbrtParseFile(std::string filename);
}

#endif