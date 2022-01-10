#include "memory.h"
#include <malloc.h>

namespace pbrt
{
	void* AllocAligned(size_t size)
	{
#if defined(PBRT_IS_WINDOWS)
		return _aligned_malloc(size, PBRT_L1_CACHE_LINE_SIZE);
#elif defined(PBRT_IS_OPENBSD) || defined(PBRT_IS_OSX)
		void* ptr;
		if (posix_memalign(&ptr, PBRT_L1_CACHE_LINE_SIZE, size) != 0)
			ptr = nullptr;
		return ptr;
#else
		return memalign(PBRT_L1_CACHE_LINE_SIZE, size);
#endif
	}

	void FreeAligned(void* ptr)
	{
		if (!ptr) return;
#if defined(PBRT_HAVE__ALIGNED_MALLOC)
		_aligned_free(ptr);
#else
		free(ptr);
#endif
	}
}
