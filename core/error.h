#ifndef PBRT_CORE_ERROR_H
#define PBRT_CORE_ERROR_H
#include "fmt/core.h"

namespace pbrt
{
	template <typename... T>
	void Info(const char* format, T&&... args)
	{
		fmt::print("Info: ");
		//fmt::print((std::string)format, &args...);
	}

	template <typename... T>
	void Warning(const char* format, T&&... args)
	{
		fmt::print("Warning: ");
		//fmt::print((std::string)format, &args...);
	}
	
	template <typename... T>
	void Error(const char* format, T&&... args)
	{
		fmt::print("Error: ");
		//fmt::print((std::string)format, &args...);
	}

}

#endif // !PBRT_CORE_ERROR_H
