#ifndef PBRT_CORE_EFLOAT_H
#define PBRT_CORE_EFLOAT_H

#include "pbrt.h"

namespace pbrt
{
	class EFloat
	{
	public:
		EFloat() = default;
		EFloat(float v, float err = 0.f);
		explicit operator float() const;
		explicit operator double() const;
		EFloat operator*(const EFloat&) const;
		EFloat operator+(const EFloat&) const;
		EFloat operator-(const EFloat&) const;
		bool operator==(const EFloat&) const;
		float UpperBound() const;
		float LowerBound() const;
	};
	inline EFloat operator*(float f, EFloat fe) { return EFloat(f) * fe; }
	inline bool Quadratic(EFloat A, EFloat B, EFloat C, EFloat* t0, EFloat* t1);
}

#endif
