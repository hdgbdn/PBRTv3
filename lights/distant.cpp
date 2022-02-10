#include "distant.h"
#include "core/scene.h"

namespace pbrt
{
	DistantLight::DistantLight(const Transform& LightToWorld, const Spectrum& L, const Vector3f& wLight)
		:Light((int)LightFlags::DeltaDirection, LightToWorld, MediumInterface()),
	L(L), wLight(Normalize(LightToWorld(wLight)))
	{
	}
	void DistantLight::Preprocess(const Scene& scene)
	{
		scene.Worldbound().BoundingSphere(&worldCenter, &worldRadius);
	}
	Spectrum DistantLight::Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const
	{
		*wi = wLight;
		*pdf = 1;
		Point3f pOutSide = ref.p + wLight * 2 * worldRadius;
		*vis = VisibilityTester(ref, Interaction(pOutSide, ref.time,
			mediumInterface));
		return L;
	}
	Spectrum DistantLight::Power() const
	{
		return L * Pi * worldRadius * worldRadius;
	}
}