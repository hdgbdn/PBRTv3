#ifndef PBRT_CORE_MATERIAL_H
#define PBRT_CORE_MATERIAL_H

#include "texture.h"

namespace pbrt
{
	enum class TransportMode { Radiance, Importance };

	class Material
	{
	public:
		virtual ~Material();
		virtual void ComputeScatteringFunctions(SurfaceInteraction* si,
		                                        MemoryArena& arena,
		                                        TransportMode mode,
		                                        bool allowMultipleLobes) const = 0;
		static void Bump(const std::shared_ptr<Texture<float>>& d,
		                 SurfaceInteraction* si);
	};
}

#endif
