#ifndef PBRT_CORE_ERROR_H
#define PBRT_CORE_ERROR_H

namespace pbrt
{
	void Info(const char* format, ...);
	void Warning(const char* format, ...);
	void Error(const char* format, ...);
}

#endif // !PBRT_CORE_ERROR_H
