#include <atomic>         // std::atomic
#include <thread>         // std::thread
#include <vector>         // std::vector
#include "fmt/core.h"
#include <string>

#include "parallel.h"

int main()
{
	pbrt::ParallelFor(
		[&](int i)
		{
			std::ostringstream id;
			id << std::this_thread::get_id();
			fmt::print("current trunk {}: in thread: {}\n", i, id.str());
		}, 16);
	pbrt::ParallelCleanup();
}