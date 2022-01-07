#include "paramset.h"

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
}