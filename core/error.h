#ifndef PBRT_CORE_ERROR_H
#define PBRT_CORE_ERROR_H

#include "pbrt.h"

namespace pbrt
{
	void Warning(const char* format, ...);
	void Error(const char* format, ...);
}

#endif // !PBRT_CORE_ERROR_H
