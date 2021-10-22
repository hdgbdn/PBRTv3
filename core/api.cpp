#include "api.h"
#include "spectrum.h"

namespace pbrt
{
	void pbrtInit(const Options& opt)
	{
		SampledSpectrum::Init();
	}
	void pbrtCleanUp(){}
}

