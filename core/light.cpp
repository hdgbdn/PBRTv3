#include "light.h"

namespace pbrt
{
	Light::Light(int flags, const Transform& LightToWorld, const MediumInterface& mediumInterface, int nSamples)
		: flags(flags), nSamples(std::max(1, nSamples)),
	mediumInterface(mediumInterface), LightToWorld(LightToWorld), WorldToLight(Inverse(LightToWorld))
	{
	}
}
