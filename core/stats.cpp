#include "stats.h"

namespace pbrt
{
	// Statistics Local Variables

	std::vector<std::function<void(StatsAccumulator&)>>* StatRegisterer::funcs;
	static StatsAccumulator statsAccumulator;
	static std::unique_ptr<std::atomic<uint64_t>[]> profileSamples;

	void StatRegisterer::CallCallbacks(StatsAccumulator& accum) {
		for (auto func : *funcs) func(accum);
	}
	void ReportThreadStats() {
		static std::mutex mutex;
		std::lock_guard<std::mutex> lock(mutex);
		StatRegisterer::CallCallbacks(statsAccumulator);
	}
	void InitProfiler()
	{
		profileSamples.reset(new std::atomic<uint64_t>[1 << (int)Prof::NumProfEvents]);
		for (int i = 0; i < (1 << (int)Prof::NumProfEvents); ++i)
			profileSamples[i] = 0;

#ifndef PBRT_IS_WINDOWS
		
#endif
	}
}