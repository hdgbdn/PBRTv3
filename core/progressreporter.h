#ifndef PBRT_CORE_PROGRESSREPORTER_H
#define PBRT_CORE_PROGRESSREPORTER_H

#include "pbrt.h"

namespace pbrt
{
	class ProgressReporter
	{
	public:
		ProgressReporter(int64_t totalWork, const std::string& title);
		void Update(int64_t num = 1);
		void Done();
	private:
	};
}

#endif
