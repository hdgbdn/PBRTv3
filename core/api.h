#ifndef PBRT_CORE_API_H
#define PBRT_CORE_API_H

#include "pbrt.h"

namespace pbrt
{
	void pbrtIdentity();
	void pbrtTranslate(float dx, float dy, float dz);
	void pbrtRotate(float angle, float ax, float ay, float az);
	void pbrtScale(float sx, float sy, float sz);
	void pbrtLookAt(float ex, float ey, float ez,
		float lx, float ly, float lz,
		float ux, float uy, float uz);
	void pbrtConcatTransform(float transform[16]);
	void pbrtTransform(float transform[16]);
	void pbrtCoordinateSystem(const std::string& name);
	void pbrtCoordSysTransform(const std::string& name);
	void pbrtActiveTransformAll();
	void pbrtActiveTransformEndTime();
	void pbrtActiveTransformStartTime();
	void pbrtTransformTimes(float start, float end);
	void pbrtPixelFilter(const std::string& name, const ParamSet& params);
	void pbrtFilm(const std::string& type, const ParamSet& params);
	void pbrtSampler(const std::string& name, const ParamSet& params);
	void pbrtAccelerator(const std::string& name, const ParamSet& params);
	void pbrtIntegrator(const std::string& name, const ParamSet& params);
	void pbrtInit(const Options& opt);
	void pbrtCleanUp();
	void pbrtParseFile(std::string filename);
}

#endif