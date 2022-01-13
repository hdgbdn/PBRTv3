#include "parallel.h"
#include "stats.h"
#include <condition_variable>

namespace pbrt
{
	// Parallel Local Definitions
	static std::vector<std::thread> threads;
	static bool shutdownThreads = false;
	class ParallelForLoop;
	static ParallelForLoop* workList = nullptr;
	static std::mutex workListMutex;
	static std::condition_variable workListCondition;
	thread_local int ThreadIndex;
	thread_local uint32_t ProfilerState;
	int MaxThreadIndex();
	static int nThreads = 10;

	class ParallelForLoop
	{
	public:
		ParallelForLoop(std::function<void(int)> func1D,
		                int64_t maxIndex, int chunkSize, int profilerState)
			: func1D(std::move(func1D)), maxIndex(maxIndex), chunkSize(chunkSize), profilerState(profilerState)
		{
		}

	public:
		std::function<void(int)> func1D;
		const int64_t maxIndex;
		const int chunkSize, profilerState;
		int64_t nextIndex = 0;
		int activeWorkers = 0;
		ParallelForLoop* next = nullptr;

		bool Finished() const
		{
			return nextIndex >= maxIndex && activeWorkers == 0;
		}
	};

	static void workerThreadFunc(int tIndex)
	{
		ThreadIndex = tIndex;
		std::unique_lock<std::mutex> lock(workListMutex);
		while(!shutdownThreads)
		{
			if(!workList)
			{
				workListCondition.wait(lock);
			}
			else
			{
				ParallelForLoop& loop = *workList;
				int64_t indexStart = loop.nextIndex;
				int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);
				loop.nextIndex = indexEnd;
				if (loop.nextIndex == loop.maxIndex)
					workList = loop.next;
				loop.activeWorkers++;
				lock.unlock();
				for (int index = indexStart; index < indexEnd; ++index)
				{
					if (loop.func1D) loop.func1D(index);
				}
				lock.lock();
				loop.activeWorkers--;
				if (loop.Finished()) workListCondition.notify_all();
			}
		}
		ReportThreadStats();
	}

	int MaxThreadIndex()
	{
		//if (PbrtOptions.nThreads != 1)
		// only for test
		if (nThreads != 1)
		{
			if (threads.empty())
			{
				ThreadIndex = 0;
				for (int i = 0; i < NumSystemCores() - 1; ++i)
					threads.emplace_back(std::thread(workerThreadFunc, i + 1));
			}
		}
		return 1 + threads.size();
	}

	int NumSystemCores()
	{
		return std::max(1u, std::thread::hardware_concurrency());
	}

	void ParallelFor(const std::function<void(int)>& func, int count, int chunkSize)
	{
		//if (PbrtOptions.nThreads == 1 || count < chunkSize)
		if (nThreads == 1 || count < chunkSize)
			for (int i = 0; i < count; ++i)
				func(i);
		if(threads.empty())
		{
			ThreadIndex = 0;
			for (int i = 0; i < NumSystemCores() - 1; ++i)
				threads.emplace_back(std::thread(workerThreadFunc, i + 1));
		}

		ParallelForLoop loop(func, count, chunkSize, CurrentProfilerState());
		workListMutex.lock();
		loop.next = workList;
		workList = &loop;
		workListMutex.unlock();

		std::unique_lock<std::mutex> lock(workListMutex);
		workListCondition.notify_all();

		while(!loop.Finished())
		{
			int64_t indexStart = loop.nextIndex;
			int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);
			loop.nextIndex = indexEnd;
			if (loop.nextIndex == loop.maxIndex)
				workList = loop.next;
			loop.activeWorkers++;
			lock.unlock();
			for (int index = indexStart; index < indexEnd; ++index)
			{
				if (loop.func1D) loop.func1D(index);
			}
			lock.lock();
			loop.activeWorkers--;
		}
	}

	void ParallelFor2D(std::function<void(Point2i)> func, const Point2i& count)
	{

	}

	void ParallelCleanup() {
		if (threads.empty()) return;

		{
			std::lock_guard<std::mutex> lock(workListMutex);
			shutdownThreads = true;
			workListCondition.notify_all();
		}

		for (std::thread& thread : threads) thread.join();
		threads.erase(threads.begin(), threads.end());
		shutdownThreads = false;
	}

}