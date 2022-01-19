#include "paramset.h"
#include "error.h"
#include "spectrum.h"
#include "constant.h"
#include "fileutil.h"

namespace pbrt
{
#define ADD_PARAM_TYPE(T, vec) \
    (vec).emplace_back(new ParamSetItem<T>(name, std::move(values), nValues));
#define LOOKUP_PTR(vec)             \
    for (const auto &v : vec)       \
        if (v->name == name) {      \
            *nValues = v->nValues;  \
            v->lookedUp = true;     \
            return v->values.get(); \
        }                           \
    return nullptr
#define LOOKUP_ONE(vec)                           \
    for (const auto &v : vec)                     \
        if (v->name == name && v->nValues == 1) { \
            v->lookedUp = true;                   \
            return v->values[0];                  \
        }                                         \
    return d

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

    Spectrum ParamSet::FindOneSpectrum(const std::string &name, const Spectrum &d) const
    {
        LOOKUP_ONE(spectra);
    }

    std::string ParamSet::FindOneString(const std::string &name, const std::string &d) const
    {
        LOOKUP_ONE(strings);
    }

    std::string ParamSet::FindOneFilename(const std::string & name, const std::string &d) const
    {
        std::string filename = FindOneString(name, "");
        if (filename.empty()) return d;
        filename = AbsolutePath(ResolveFilename(filename));
        return filename;
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

    std::string ParamSet::FindTexture(const std::string& name) const
    {
        std::string d = "";
        LOOKUP_ONE(textures);
    }

    const float *ParamSet::FindFloat(const std::string & name, int *n) const
    {
        for (const auto& f : floats)
            if (f->name == name)
            {
                *n = f->nValues;
                f->lookedUp = true;
                return f->values.get();
            }
        return nullptr;
    }

    const Spectrum *ParamSet::FindSpectrum(const std::string& name, int *nValues) const
    {
        LOOKUP_PTR(spectra);
    }

    void ParamSet::ReportUnused() const
    {
        // TODO implement
    }

    void ParamSet::AddInt(const std::string &name, std::unique_ptr<int[]> values, int nValues)
    {
        EraseInt(name);
        ADD_PARAM_TYPE(int, ints);
    }

    bool ParamSet::EraseInt(const std::string & n)
    {
        for (size_t i = 0; i < ints.size(); ++i)
            if (ints[i]->name == n)
            {
                ints.erase(ints.begin() + i);
                return true;
            }
        return false;
    }

    void ParamSet::AddFloat(const std::string &name, std::unique_ptr<float[]> values, int nValues)
    {
        EraseFloat(name);
        ADD_PARAM_TYPE(float, floats);
    }

    bool ParamSet::EraseFloat(const std::string & n)
    {
        for (size_t i = 0; i < floats.size(); ++i)
            if (floats[i]->name == n)
            {
                floats.erase(floats.begin() + i);
                return true;
            }
        return false;
    }

    void ParamSet::AddString(const std::string & name, std::unique_ptr<std::string[]> values, int nValues)
    {
        EraseString(name);
        ADD_PARAM_TYPE(std::string , strings);
    }

    bool ParamSet::EraseString(const std::string &name)
    {
        for (size_t i = 0; i < strings.size(); ++i)
        {
            if(strings[i]->name == name)
            {
                strings.erase(strings.begin() + i);
                return true;
            }
        }
        return false;
    }

    void ParamSet::AddRGBSpectrum(const std::string & name, std::unique_ptr<float[]> values, int nValues)
    {
        EraseSpectrum(name);
        // CHECK_EQ(nValues % 3, 0);
        nValues /= 3;
        std::unique_ptr<Spectrum[]> s(new Spectrum[nValues]);
        for (int i = 0; i < nValues; ++i) s[i] = Spectrum::FromRGB(&values[3 * i]);
        std::shared_ptr<ParamSetItem<Spectrum>> psi(
                new ParamSetItem<Spectrum>(name, std::move(s), nValues));
        spectra.push_back(psi);
    }

    bool ParamSet::EraseSpectrum(const std::string & n)
    {
        for (size_t i = 0; i < spectra.size(); ++i)
        {
            if(spectra[i]->name == n)
            {
                spectra.erase(spectra.begin() + i);
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<Texture<Spectrum>> TextureParams::GetSpectrumTexture(const std::string& n, const Spectrum& def) const
	{
        std::shared_ptr<Texture<Spectrum>> tex = GetSpectrumTextureOrNull(n);
        if (tex)
            return tex;
        else
            return std::make_shared<ConstantTexture<Spectrum>>(def);
	}

    std::shared_ptr<Texture<Spectrum>> TextureParams::GetSpectrumTextureOrNull(const std::string &n) const
    {
        std::string name = geoParams.FindTexture(n);
        if (name.empty())
        {
            int count;
            const Spectrum* s = geoParams.FindSpectrum(n, &count);
            if (s)
            {
                if (count > 1)
                    Warning("Ignoring excess values provided with parameter \"%s\"",
                            n.c_str());
                return std::make_shared<ConstantTexture<Spectrum>>(*s);
            }

            name = materialParams.FindTexture(n);
            if (name.empty())
            {
                s = materialParams.FindSpectrum(n, &count);
                if (s)
                {
                    if (count > 1)
                        Warning("Ignoring excess values provided with parameter \"%s\"",
                                          n.c_str());
                    return std::make_shared<ConstantTexture<Spectrum>>(*s);
                }
            }
            if (name.empty()) return nullptr;
        }

        if(spectrumTextures.find(name) != spectrumTextures.end())
            return spectrumTextures[name];
        else
        {
            Error("Couldn't find spectrum texture named \"%s\" for parameter \"%s\"",
                  name.c_str(), n.c_str());
            return nullptr;
        }
    }

    std::shared_ptr<Texture<float>> TextureParams::GetFloatTexture(const std::string &name, float def) const
    {
        std::shared_ptr<Texture<float>> tex = GetFloatTextureOrNull(name);
        if (tex)
            return tex;
        else
            return std::make_shared<ConstantTexture<float>>(def);
    }

    std::shared_ptr<Texture<float>> TextureParams::GetFloatTextureOrNull(const std::string &n) const
    {
        std::string name = geoParams.FindTexture(n);
        if (name.empty())
        {
            int count;
            const float* s = geoParams.FindFloat(n, &count);
            if (s)
            {
                Warning("Ignoring excess values provided with parameter \"%s\"",
                        n.c_str());
                return std::make_shared<ConstantTexture<float>>(*s);
            }

            name = materialParams.FindTexture(n);
            if (name.empty())
            {
                s = materialParams.FindFloat(n, &count);
                if (s) {
                    if (count > 1)
                        Warning("Ignoring excess values provided with parameter \"%s\"",
                                n.c_str());
                    return std::make_shared<ConstantTexture<float>>(*s);
                }
            }

            if (name.empty())
                return nullptr;
        }

        if (floatTextures.find(name) != floatTextures.end())
            return floatTextures[name];
        else {
            Error("Couldn't find float texture named \"%s\" for parameter \"%s\"",
                  name.c_str(), n.c_str());
            return nullptr;
        }
    }

    void TextureParams::ReportUnused() const
    {
        // TODO need implement
    }
}