#ifndef PBRT_CORE_MIPMAP_H
#define PBRT_CORE_MIPMAP_H

#include "pbrt.h"
#include "parallel.h"
#include "memory.h"

namespace pbrt
{
	enum class ImageWrap { Repeat, Black, Clamp };
	struct ResampleWeight
	{
		int firstTexel;
		float weight[4];
	};
	template <typename T>
	class MIPMap
	{
	public:
		MIPMap(const Point2i& res, const T* img, bool doTrilinear = false, float maxAnisotropy = 8.f, ImageWrap wrapMode = ImageWrap::Repeat);
		int Width() const { return resolution.x; }
		int Height() const { return resolution.y; }
		int Levels() const { return pyramid.size(); }
		const T& Texel(int level, int s, int t) const;
		T Lookup(const Point2f& st, float width = 0.f) const;
		T triangle(int level, const Point2f& st) const;
	private:
		const bool doTrilinear;
		const float maxAnisotropy;
		const ImageWrap wrapMode;
		Point2i resolution;
		std::vector<std::unique_ptr<BlockedArray<T>>> pyramid;
		std::unique_ptr<ResampleWeight[]> resampleWeights(int oldRes, int newRes);
	};
}

#endif