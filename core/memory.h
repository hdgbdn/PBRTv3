#ifndef PBRT_CORE_MEMORY_H
#define PBRT_CORE_MEMORY_H

namespace pbrt
{
	class MemoryArena
	{
	public:
		MemoryArena(size_t blockSize = 262144) : blockSize(blockSize) {}
		template <typename T>
		T* Alloc(size_t n = 1, bool runConstructor = true) {
			T* ret = (T*)Alloc(n * sizeof(T));
			if (runConstructor)
				for (size_t i = 0; i < n; ++i) new (&ret[i]) T();
			return ret;
		}
		void Reset();
	private:
		MemoryArena(const MemoryArena&) = delete;
		MemoryArena& operator=(const MemoryArena&) = delete;
		const size_t blockSize;
	};
}

#endif