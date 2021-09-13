#ifndef PBRT_CORE_LIGHT_H
#define PBRT_CORE_LIGHT_H

#include "pbrt.h"

namespace pbrt
{
	class Light
	{
	public:
		void Preprocess(const Scene& scene);
	};
}

#endif