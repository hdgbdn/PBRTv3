#ifndef PBRT_CORE_PARALLEL_H
#define PBRT_CORE_PARALLEL_H

#include "pbrt.h"
#include <functional>

namespace pbrt
{
    class AtomicFloat {
    public:
        // AtomicFloat Public Methods
        explicit AtomicFloat(float v = 0) { bits = FloatToBits(v); }
        operator float() const { return BitsToFloat(bits); }
        float operator=(float v) {
            bits = FloatToBits(v);
            return v;
        }
        void Add(float v) {
            uint32_t oldBits = bits, newBits;
            do {
                newBits = FloatToBits(BitsToFloat(oldBits) + v);
            } while (!bits.compare_exchange_weak(oldBits, newBits));
        }

    private:
        // AtomicFloat Private Data
        std::atomic<uint32_t> bits;
    };

    extern thread_local int ThreadIndex;
    int NumSystemCores();
    void ParallelInit();
    void ParallelCleanup();

	void ParallelFor(const std::function<void(int)>& func, int count,
	                 int chunkSize = 1);
}



#endif