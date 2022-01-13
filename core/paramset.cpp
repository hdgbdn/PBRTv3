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

    int ParamSet::FindOneInt(const std::string &name, int d) const
    {
        return 0;
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

    Point2f ParamSet::FindOnePoint2f(const std::string &name, const Point2f &d) const
    {
        return pbrt::Point2f();
    }

    Vector2f ParamSet::FindOneVector2f(const std::string &name, const Vector2f &d) const
    {
        return pbrt::Vector2f();
    }

    Point3f ParamSet::FindOnePoint3f(const std::string &name, const Point3f &d) const
    {
        return pbrt::Point3f();
    }

    Normal3f ParamSet::FindOneNormal3f(const std::string &name, const Normal3f &d) const
    {
        return pbrt::Normal3f();
    }

    std::string ParamSet::FindOneString(const std::string &name, const std::string &d) const
    {
        return std::string();
    }

    Spectrum ParamSet::FindOneSpectrum(const std::string &name, const Spectrum &d) const
    {
        return pbrt::Spectrum();
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

    std::string ParamSet::FindTexture(const std::string &) const
    {
        // TODO implement
        return std::string();
    }

    void ParamSet::ReportUnused() const
    {
        // TODO implement
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