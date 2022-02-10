#include "projection.h"
#include "core/imageio.h"

namespace pbrt
{
	ProjectionLight::ProjectionLight(const Transform& LightToWorld, const MediumInterface& mediumInterface, const Spectrum& I, const std::string& texname, float fov)
		: Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
		pLight(LightToWorld(Point3f(0, 0, 0))), I(I)
	{
		Point2i resolution;
		std::unique_ptr<RGBSpectrum[]> texels = ReadImage(texname, &resolution);
		if (texels)
			projectionMap.reset(new MIPMap<RGBSpectrum>(resolution, texels.get()));
		float aspect = projectionMap ?
			(float(resolution.x) / float(resolution.y)) : 1;
		if (aspect > 1)
			screenBounds = Bounds2f(Point2f(-aspect, -1), Point2f(aspect, 1));
		else
			screenBounds = Bounds2f(Point2f(-1, -1 / aspect), Point2f(1, 1 / aspect));
		near = 1e-3f;
		far = 1e30f;
		lightProjection = Perspective(fov, near, far);
		float opposite = std::tan(Radians(fov) / 2);
		float tanDiag = opposite * std::sqrt(1 + 1 / (aspect * aspect));
		cosTotalWidth = std::cos(std::atan(tanDiag));
	}
	Spectrum ProjectionLight::Projection(const Vector3f& w) const
	{
		Vector3f wl = WorldToLight(w);
		if (wl.z < near) return 0.f;
		Point3f p = lightProjection(Point3f(wl.x, wl.y, wl.z));
		if (!Inside(Point2f(p.x, p.y), screenBounds)) return 0.f;
		if (!projectionMap) return 1;
		Point2f st = Point2f(screenBounds.Offset(Point2f(p.x, p.y)));
		return Spectrum(projectionMap->Lookup(st), SpectrumType::Illuminant);
	}
	Spectrum ProjectionLight::Power() const
	{
		return (projectionMap ?
			Spectrum(projectionMap->Lookup(Point2f(.5f, .5f), .5f),
				SpectrumType::Illuminant) : Spectrum(1.f)) *
			I * 2 * Pi * (1.f - cosTotalWidth);
	}
}
