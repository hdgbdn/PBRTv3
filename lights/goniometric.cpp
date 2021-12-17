#include "goniometric.h"

namespace pbrt
{
	GonioPhotometricLight::GonioPhotometricLight(const Transform& LightToWorld, const MediumInterface& mediumInterface,
	                                             const Spectrum& I, const std::string& texName)
		: Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
		  pLight(LightToWorld(Point3f(0, 0, 0))), I(I)
	{
		Point2i resolution;
		std::unique_ptr<RGBSpectrum[]> texels = ReadImage(texName, &resolution);
		if (texels)
			mipmap.reset(new MIPMap<RGBSpectrum>(resolution, texels.get()));
	}
	Spectrum GonioPhotometricLight::Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const
	{
		*wi = Normalize(pLight - ref.p);
		*pdf = 1.f;
		*vis = VisibilityTester(ref, Interaction(pLight, ref.time, mediumInterface));
		return I * Scale(-*wi) / DistanceSquared(pLight, ref.p);
	}
	Spectrum GonioPhotometricLight::Scale(const Vector3f& w) const
	{
		Vector3f wp = Normalize(WorldToLight(w));
		std::swap(wp.y, wp.z);
		float theta = SphericalTheta(wp);
		float phi = SphericalPhi(wp);
		Point2f st(phi * Inv2Pi, theta * InvPi);
		return !mipmap ? RGBSpectrum(1.f) : Spectrum(mipmap->Lookup(st), SpectrumType::Illuminant);
	}
	Spectrum GonioPhotometricLight::Power() const
	{
		return 4 * Pi * I *
			Spectrum(mipmap ? mipmap->Lookup(Point2f(.5f, .5f), .5f) :
				Spectrum(1.f), SpectrumType::Illuminant);
	}
}
