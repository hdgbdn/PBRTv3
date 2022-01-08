#include "paramset.h"
#include "error.h"
#include "spectrum.h"
#include "constant.h"

namespace pbrt
{
	template <typename T>
	ParamSetItem<T>::ParamSetItem(const std::string& name, const T* v, int nValues)
		: name(name), values(new T[nValues]), nValues(nValues)
	{
		std::copy(v, v + nValues, values.get());
	}

	bool ParamSet::FindOneBool(const std::string& name, bool d) const
	{
		for (const auto& b : bools)
			if (b->name == name && b->nValues == 1) {
				b->lookedUp = true;
				return b->values[0];
			}
		return d;
	}

	float ParamSet::FindOneFloat(const std::string& name, float d) const
	{
		for(const auto& f : floats)
			if (f->name == name && f->nValues == 1) {
				f->lookedUp = true;
				return f->values[0];
			}
		return d;
	}

	void ParamSet::Clear()
	{
		bools.clear();
		ints.clear();
		floats.clear();
		point2fs.clear();
		vector2fs.clear();
		point3fs.clear();
		vector3fs.clear();
		normal3fs.clear();
		spectra.clear();
		strings.clear();
		textures.clear();
	}
	std::shared_ptr<Texture<Spectrum>> TextureParams::GetSpectrumTexture(const std::string& n, const Spectrum& d) const
	{
		std::string name = geoParams.FindTexture(n);
		if (name.empty()) name = materialParams.FindTexture(n);
		if (!name.empty())
		{
			if (spectrumTextures.find(name) != spectrumTextures.end())
				return spectrumTextures[name];
			else
				Error("Couldn't find spectrum texture named \"%s\" "
					"for parameter \"%s\"", name.c_str(), n.c_str());
		}
		Spectrum val = materialParams.FindOneSpectrum(n, d);
		val = geoParams.FindOneSpectrum(n, val);
		return std::make_shared<ConstantTexture<Spectrum>>(val);
	}
}