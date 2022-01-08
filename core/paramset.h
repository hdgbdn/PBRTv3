#ifndef PBRT_CORE_PARAMSET_H
#define PBRT_CORE_PARAMSET_H

#include "pbrt.h"
#include "geometry.h"

#include <map>
#include <memory>

namespace pbrt
{
	template<typename T>
	struct ParamSetItem
	{
		ParamSetItem(const std::string& name, const T* v, int nValues);
		const std::string name;
		const std::unique_ptr<T[]> values;
		const int nValues;
		mutable bool lookedUp = false;
	};

	class ParamSet
	{
	public:
		ParamSet() = default;
		void AddBool(const std::string& name, const bool* values, int nValues);
		void AddInt(const std::string& name, const int* values, int nValues);
		void AddFloat(const std::string& name, const float* values, int nValues);
		void AddPoint2f(const std::string& name, const Point2f* values, int nValues);
		void AddVector2f(const std::string& name, const Vector2f* values, int nValues);
		void AddPoint3f(const std::string& name, const Point3f* values, int nValues);
		void AddNormal3f(const std::string& name, const Normal3f* values, int nValues);
		void AddString(const std::string& name, const std::string* values, int nValues);
		void AddRGBSpectrum(const std::string& name, const float*, int nValues);
		void AddXYZSpectrum(const std::string& name, const float*, int nValues);
		void AddBlackbodySpectrum(const std::string& name, const float*, int nValues);
		void AddSampledSpectrumFiles(const std::string& name, const float**, int nValues);
		void AddSampledSpectrum(const std::string& name, const float*, int nValues);

		template <typename T>
		T FindOne(const std::string& name, const T& d) const
		{
			// TODO how to find type's vector?
			
			return d;
		}
		bool FindOneBool(const std::string& name, bool d) const;
		int FindOneInt(const std::string& name, int d) const;
		float FindOneFloat(const std::string& name, float d) const;
		Point2f FindOnePoint2f(const std::string& name, const Point2f& d) const;
		Vector2f FindOneVector2f(const std::string& name, const Vector2f& d) const;
		Point3f FindOnePoint3f(const std::string& name, const Point3f& d) const;
		Normal3f FindOneNormal3f(const std::string& name, const Normal3f& d) const;
		std::string FindOneString(const std::string& name, const std::string& d) const;
		Spectrum FindOneSpectrum(const std::string& name, const Spectrum& d) const;
		std::string FindTexture(const std::string&) const;

		void ReportUnused() const;
		void Clear();
	private:
		std::vector<std::shared_ptr<ParamSetItem<bool>>> bools;
		std::vector<std::shared_ptr<ParamSetItem<int>>> ints;
		std::vector<std::shared_ptr<ParamSetItem<float>>> floats;
		std::vector<std::shared_ptr<ParamSetItem<Point2f>>> point2fs;
		std::vector<std::shared_ptr<ParamSetItem<Vector2f>>> vector2fs;
		std::vector<std::shared_ptr<ParamSetItem<Point3f>>> point3fs;
		std::vector<std::shared_ptr<ParamSetItem<Vector3f>>> vector3fs;
		std::vector<std::shared_ptr<ParamSetItem<Normal3f>>> normal3fs;
		std::vector<std::shared_ptr<ParamSetItem<Spectrum>>> spectra;
		std::vector<std::shared_ptr<ParamSetItem<std::string>>> strings;
		std::vector<std::shared_ptr<ParamSetItem<std::string>>> textures;
	};

	class TextureParams
	{
	public:
		TextureParams(const ParamSet& geoParams,
		              const ParamSet& materialParams,
		              std::map<std::string, std::shared_ptr<Texture<float>>>& floatTextures,
		              std::map<std::string, std::shared_ptr<Texture<Spectrum>>>& spectrumTextures)
			:
			geoParams(geoParams),
			materialParams(materialParams),
			floatTextures(floatTextures),
			spectrumTextures(spectrumTextures)
		{
		}
		std::shared_ptr<Texture<Spectrum>> GetSpectrumTexture(const std::string& n, const Spectrum& d) const;
	private:
		std::map<std::string, std::shared_ptr<Texture<float>>>& floatTextures;
		std::map<std::string, std::shared_ptr<Texture<Spectrum>>>& spectrumTextures;
		const ParamSet& geoParams, & materialParams;
	};
}

#endif