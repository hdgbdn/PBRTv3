#ifndef PBRT_CORE_MATERIAL_H
#define PBRT_CORE_MATERIAL_H

#include "pbrt.h"

namespace pbrt
{
	enum class TransportMode { Radiance, Importance };
	class Material
	{
	public:
		virtual ~Material() = default;
		virtual void ComputeScatteringFunctions(SurfaceInteraction* si,
		                                        MemoryArena& arena,
		                                        TransportMode mode,
		                                        bool allowMultipleLobes) const = 0;
	};
}

#endif