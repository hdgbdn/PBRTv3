#include "matte.h"
#include "memory.h"
#include "reflection.h"
#include "texture.h"

namespace pbrt
{
	void MatteMaterial::ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena, TransportMode mode,
	                                               bool allowMultipleLobes) const
	{
		if (bumpMap)
			Bump(bumpMap, si);
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
		Spectrum r = Kd->Evaluate(*si).Clamp();
		float sig = Clamp(sigma->Evaluate(*si), 0, 90);
		if (!r.IsBlack()) {
			if (sig == 0)
				si->bsdf->Add(ARENA_ALLOC(arena, LambertianReflection)(r));
			else
				si->bsdf->Add(ARENA_ALLOC(arena, OrenNayar)(r, sig));
		}
	}

}
