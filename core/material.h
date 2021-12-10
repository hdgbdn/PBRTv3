#ifndef PBRT_CORE_MATERIAL_H
#define PBRT_CORE_MATERIAL_H

#include "geometry.h"
#include "pbrt.h"
#include "interaction.h"
#include "texture.h"

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
        static void Bump(const std::shared_ptr<Texture<float>>& d,
            SurfaceInteraction* si)
		{
			SurfaceInteraction siEval = *si;
			float du = .5f * (std::abs(si->dudx) + std::abs(si->dudy));
			if (du == 0) du = .01f;
			siEval.p = si->p + du * si->shading.dpdu;
			siEval.uv = si->uv + Vector2f(du, 0.f);
			siEval.n = Normalize((Normal3f)Cross(si->shading.dpdu,
				si->shading.dpdv) +
				du * si->dndu);
			float uDisplace = d->Evaluate(siEval);
			float dv = .5f * (std::abs(si->dvdx) + std::abs(si->dvdy));
			if (dv == 0) dv = .01f;
			siEval.p = si->p + dv * si->shading.dpdv;
			siEval.uv = si->uv + Vector2f(0.f, dv);
			siEval.n = Normalize((Normal3f)Cross(si->shading.dpdu,
				si->shading.dpdv) +
				dv * si->dndv);
			float vDisplace = d->Evaluate(siEval);
			float displace = d->Evaluate(*si);
			Vector3f dpdu = si->shading.dpdu +
				(uDisplace - displace) / du * Vector3f(si->shading.n) +
				displace * Vector3f(si->shading.dndu);
			Vector3f dpdv = si->shading.dpdv +
				(vDisplace - displace) / dv * Vector3f(si->shading.n) +
				displace * Vector3f(si->shading.dndv);
			si->SetShadingGeometry(dpdu, dpdv, si->shading.dndu, si->shading.dndv,
				false);
        }
	};
}

#endif