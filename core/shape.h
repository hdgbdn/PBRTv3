#ifndef PBRT_CORE_SHAPE_H
#define PBRT_CORE_SHAPE_H

#include "pbrt.h"

namespace pbrt
{
	class Shape
	{
	public:
		const bool reverseOrientation;
		const bool transformSwapsHandedness;
	};
}

#endif