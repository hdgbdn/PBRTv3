#include "memory.h"
#include <malloc.h>

namespace pbrt
{
	void* AllocAligned(size_t size)
	{
		return _aligned_malloc(size, PBRT_L1_CACHE_LINE_SIZE);
	}

	void FreeAligned(void* ptr)
	{
		if (!ptr) return;
		_aligned_free(ptr);
	}
}
