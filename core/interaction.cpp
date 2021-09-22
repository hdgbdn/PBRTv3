#include <interaction.h>

namespace pbrt
{
	SurfaceInteraction::SurfaceInteraction(const Point3f& p, const Vector3f& pError,
		const Point2f& uv, const Vector3f& wo,
		const Vector3f& dpdu, const Vector3f& dpdv,
		const Normal3f& dndu, const Normal3f& dndv,
		float time, const std::shared_ptr<Shape> shape):
		Interaction(p, Normal3f(Normalize(Cross(dpdu, dpdv))),
			pError, wo, time, nullptr), uv(uv),
	dpdu(dpdu),
	dpdv(dpdv),
	dndu(dndu),
	dndv(dndv),
	shape(shape)
	{
		shading.n = n;
		shading.dpdu = dpdu;
		shading.dpdv = dpdv;
		shading.dndu = dndu;
		shading.dndv = dndv;
		if(shape != nullptr &&(shape->reverseOrientation ^ 
			shape->transformSwapsHandedness) )
		{
			n *= -1;
			shading.n *= -1;
		}
	}

	void SurfaceInteraction::SetShadingGeometry(const Vector3f& dpdus, const Vector3f& dpdvs, const Normal3f& dndus, const Normal3f& dndvs, bool orientationIsAuthoritative)
	{
		shading.n = Normalize(static_cast<Normal3f>(Cross(dpdus, dpdvs)));
		if (nullptr != shape && (shape->reverseOrientation ^
			shape->transformSwapsHandedness))
			shading.n = -shading.n;
		if (orientationIsAuthoritative)
			n = Faceforward(n, shading.n);
		else
			shading.n = Faceforward(shading.n, n);
		shading.dpdu = dpdu;
		shading.dpdv = dpdv;
		shading.dndu = dndu;
		shading.dndv = dndv;
	}


}

