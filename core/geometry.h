#ifndef PBRT_CORE_GEOMETRY_H
#define PBRT_CORE_GEOMETRY_H

#include "pbrt.h"
#include "geometry.h"

namespace pbrt
{
	template <typename T>
	class Point2
	{
	public:
		Point2(T xx, T yy);
		T x, y;
	};
	template <typename T>
	class Point3 {};

	typedef Point2<float> Point2f;
	typedef Point2<int> Point2i;
	typedef Point3<float> Point3f;
	typedef Point3<int> Point3i;

	template <typename T>
	class Bounds2
	{
	public:
		Bounds2(const Point2<T>& p1, const Point2<T>& p2);
		Vector2<T> Diagonal() const { return pMax - pMin; }
		Point2<T> pMin, pMax;
		bool operator==(const Bounds2<T>& b) const {
			return b.pMin == pMin && b.pMax == pMax;
		}
		bool operator!=(const Bounds2<T>& b) const {
			return b.pMin != pMin || b.pMax != pMax;
		}
	};
	template <typename T>
	class Bounds3{};
	
	typedef Bounds2<float> Bounds2f;
	typedef Bounds2<int> Bounds2i;
	typedef Bounds3<float> Bounds3f;
	typedef Bounds3<int> Bounds3i;


	template <typename T>
	class Vector2
	{
	public:
		T x, y;
	};
	template <typename T>
	class Vector3 {};

	typedef Vector2<float> Vector2f;
	typedef Vector2<int> Vector2i;
	typedef Vector3<float> Vector3f;
	typedef Vector3<int> Vector3i;

	class Bounds2iIterator : public std::forward_iterator_tag
	{
	public:
		Bounds2iIterator operator++();
		Bounds2iIterator operator++(int);
		bool operator==(const Bounds2iIterator &bi) const;
		bool operator!= (const Bounds2iIterator &bi) const;
		Point2i operator*() const;
	};
	inline Bounds2iIterator begin(const Bounds2i& b) {}
	inline Bounds2iIterator end(const Bounds2i& e) {}

	class Ray{};
	class RayDifferential : public Ray
	{
	public:
		void ScaleDifferentials(float s);
	};
}

#endif