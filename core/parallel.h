#ifndef PBRT_CORE_PARALLEL_H
#define PBRT_CORE_PARALLEL_H

#include "pbrt.h"
#include "geometry.h"
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
#ifdef PBRT_FLOAT_AS_DOUBLE
            uint64_t oldBits = bits, newBits;
#else
            uint32_t oldBits = bits, newBits;
#endif
            do {
                newBits = FloatToBits(BitsToFloat(oldBits) + v);
            } while (!bits.compare_exchange_weak(oldBits, newBits));
        }

    private:
        // AtomicFloat Private Data
#ifdef PBRT_FLOAT_AS_DOUBLE
        std::atomic<uint64_t> bits;
#else
        std::atomic<uint32_t> bits;
#endif
    };

	void ParallelFor2D(std::function<void(Point2i)> func, const Point2i &count);
	void ParallelFor(std::function<void(int64_t)> func, int64_t count,
		int chunkSize = 1);
}



#endif