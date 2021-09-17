#ifndef PBRT_CORE_GEOMETRY_H
#define PBRT_CORE_GEOMETRY_H

#include <cassert>
#include <iostream>
#include "pbrt.h"

namespace pbrt
{
	
	template <typename T>
	class Vector2
	{
	public:
		T operator[](int i) const
		{
			assert(i >= 0 && i <= 1);
			if (i == 0) return x;
			return y;
		}
		T& operator[](int i)
		{
			assert(i >= 0 && i <= 1);
			if (i == 0) return x;
			return y;
		}
		Vector2<T> operator+(const Vector2<T>& rhs) const
		{
			return Vector2<T>(x + rhs.x, y + rhs.y);
		}
		Vector2<T>& operator+=(const Vector2<T>& rhs)
		{
			x += rhs.x; y += rhs.y;;
			return *this;
		}
		Vector2<T> operator-(const Vector2<T>& rhs)
		{
			return Vector2<T>(x - rhs.x, y - rhs.y);
		}
		Vector2<T>& operator-=(const Vector2<T>& rhs)
		{
			x -= rhs.x; y -= rhs.y;
			return *this;
		}
		Vector2<T> operator*(T s) const
		{
			return Vector2<T>(s * x, s * y);
		}
		Vector2<T>& operator*=(T s)
		{
			x *= s; y *= s;
			return *this;
		}
		Vector2<T> operator/(T f) const
		{
			assert(f != 0);
			float inv = static_cast<float>(1 / f);
			return Vector2(x * inv, y * inv);
		}
		Vector2<T>& operator/=(T f)
		{
			assert(f != 0);
			float inv = static_cast<float>(1 / f);
			x *= inv; y *= inv;
			return *this;
		}
		Vector2<T> operator-()
		{
			return Vector2<T>(-x, -y);
		}
		float LengthSquared() const { return x * x + y * y; }
		float Length() const { return std::sqrt(LengthSquared()); }
		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y);
		}
		T x, y;
	};
	template <typename T>
	class Vector3
	{
	public:
		Vector3() : x(0), y(0), z(0) {}
		Vector3(T xx, T yy, T zz) : x(xx), y(yy), z(zz)
		{
			assert(!HasNaNs());
		}
		T operator[](size_t i) const
		{
			assert(i <= 2);
			if (i == 0) return x;
			if (i == 1) return y;
			return z;
		}
		T& operator[](size_t i)
		{
			assert(i <= 2);
			if (i == 0) return x;
			if (i == 1) return y;
			return z;
		}
		Vector3<T> operator+(const Vector3<T>& rhs) const
		{
			return Vector3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
		}
		Vector3<T>& operator+=(const Vector3<T>& rhs)
		{
			x += rhs.x; y += rhs.y; z += rhs.z;
			return *this;
		}
		Vector3<T> operator-(const Vector3<T>& rhs)
		{
			return Vector3<T>(x - rhs.x, y - rhs.y, z - rhs.z);
		}
		Vector3<T>& operator-=(const Vector3<T>& rhs)
		{
			x -= rhs.x; y -= rhs.y; z -= rhs.z;
			return *this;
		}
		Vector3<T> operator*(T s) const
		{
			return Vector3<T>(s * x, s * y, s * z);
		}
		Vector3<T>& operator*=(T s)
		{
			x *= s; y *= s; z *= s;
			return *this;
		}
		Vector3<T> operator/(T f) const
		{
			assert(f != 0);
			float inv = static_cast<float>(1 / f);
			return Vector3(x * inv, y * inv, z * inv);
		}
		Vector3<T> &operator/=(T f)
		{
			assert(f != 0);
			float inv = static_cast<float>(1 / f);
			x *= inv; y *= inv; z *= inv;
			return *this;
		}
		Vector3<T> operator-()
		{
			return Vector3<T>(-x, -y, -z);
		}
		float LengthSquared() const { return x * x + y * y + z * z; }
		float Length() const { return std::sqrt(LengthSquared()); }
		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y) || std::isnan(z);
		}
		T x, y, z;
	};

    template<typename T, typename U>
    Vector3<T>& operator*(U s, const Vector3<T> &v)
	{
		return v * s;
	}

	template<typename T>
	Vector3<T> Abs(const Vector3<T> &v)
    {
		return Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
    }

	template<typename T>
	Vector3<T> Cross(const Vector3<T> &lhs, const Vector3<T> &rhs)
    {
		double v1x = lhs.x, v1y = lhs.y, v1z = lhs.z;
		double v2x = rhs.x, v2y = rhs.y, v2z = rhs.z;
		return Vector3<T>(
			(v1y * v2z) - (v1z * v2y),
			(v1z * v2x) - (v1x * v2z),
			(v1x * v2y) - (v1y * v2x)
			);
    }

	template<typename T>
	Vector3<T> Normalize(const Vector3<T> &v)
    {
		return v / v.Length();
    }

	template<typename T>
	T MinComponent(const Vector3<T> &v)
    {
		return std::min(v.x, std::min(v.y, v.z));
    }

	template<typename T>
	T MaxComponent(const Vector3<T>& v)
	{
		return std::max(v.x, std::max(v.y, v.z));
	}

	template<typename T>
	size_t MaxDimension(const Vector3<T>& v)
	{
		return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) : ((v.y > v.z) ? 1 : 2);
	}

	template<typename T>
	size_t MinDimension(const Vector3<T>& v)
	{
		return (v.x < v.y) ? ((v.x < v.z) ? 0 : 2) : ((v.y < v.z) ? 1 : 2);
	}

	template<typename T>
	Vector3<T> Min(const Vector3<T> &lhs, const Vector3<T> &rhs)
    {
		return Vector3<T>(
			std::min(lhs.x, rhs.x),
			std::min(lhs.y, rhs.y),
			std::min(lhs.z, rhs.z)
			);
    }

	template<typename T>
	Vector3<T> Max(const Vector3<T>& lhs, const Vector3<T>& rhs)
	{
		return Vector3<T>(
			std::max(lhs.x, rhs.x),
			std::max(lhs.y, rhs.y),
			std::max(lhs.z, rhs.z)
			);
	}

	template <typename T>
	Vector3<T> Permute(const Vector3<T> &v, size_t x, size_t y, size_t z)
    {
		return Vector3<T>(v[x], v[y], v[z]);
    }

	template<typename T>
	void CoordinateSystem(const Vector3<T> &v1, Vector3<T> *v2, Vector3<T> * v3)
    {
		if (std::abs(v1.x) > std::abs(v1.y))
			*v2 = Vector3<T>(-v1.z, 0, v1.x) /
			std::sqrt(v1.x * v1.x + v1.z * v1.z);
		else
			*v2 = Vector3<T>(-v1.z, 0, v1.x) /
			std::sqrt(v1.x * v1.x + v1.z * v1.z);
		*v3 = Cross(v1, v2);
    }

	template<typename T>
	std::ostream& operator<<(std::ostream &os, const Vector3<T> &v)
	{
		os << fmt::format("[ {}, {}, {} ]", v.x, v.y, v.z);
		return os;
	}

	typedef Vector2<float> Vector2f;
	typedef Vector2<int> Vector2i;
	typedef Vector3<float> Vector3f;
	typedef Vector3<int> Vector3i;

	template <typename T>
	class Point2
	{
	public:
		Point2() = default;
		Point2(T xx, T yy): x(xx), y(yy)
		{
			assert(!HasNaNs());
		}
		Point2(const Point3<T> &p): x(p.x), y(p.y)
		{
			assert(!HasNaNs());
		}
		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y);
		}
		T x, y;
	};
	template <typename T>
	class Point3
	{
	public:
		Point3() = default;
		Point3(T xx, T yy, T zz):x(xx), y(yy), z(zz)
		{
			assert(!HasNaNs());
		}

		template<typename U>
		explicit Point3(const Point3<U> &p)
			:x(static_cast<T>(p.x)), y(static_cast<T>(p.y)), z(static_cast<T>(p.z))
		{
			assert(!HasNaNs());
		}

		template<typename U>
		explicit operator Vector3<U>() const
		{
			return Vector3<U>(x, y, z);
		}

		T operator[](size_t i) const
		{
			assert(i <= 2);
			if (i == 0) return x;
			if (i == 1) return y;
			return z;
		}

		T& operator[](size_t i)
		{
			assert(i <= 2);
			if (i == 0) return x;
			if (i == 1) return y;
			return z;
		}

		Point3<T> operator+(const Vector3<T> &v) const
		{
			return Point3<T>(x + v.x, y + v.y, z + v.z);
		}

		Point3<T> &operator+=(const Vector3<T> &v)
		{
			x += v.x; y += v.y; z += v.z;
			return *this;
		}

		Vector3<T> operator-(const Point3<T> &p) const
		{
			return Vector3<T>(x - p.x, y - p.y, z - p.z);
		}

		Point3<T> operator-(const Vector3<T> &v) const
		{
			return Point3<T>(x - v.x, y - v.y, z - v.z);
		}

		Point3<T>& operator-=(const Vector3<T>& v)
		{
			x -= v.x; y -= v.y; z -= v.z;
			return *this;
		}

		Point3<T> operator+(const Point3<T> &rhs)
		{
			return Point3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
		}

		template<typename U>
		Point3<T> operator*(U s) const
		{
			return Point3<T>(s * x, s * y, s * z);
		}

		template<typename U>
		Point3<T>& operator*=(U s)
		{
			x *= s; y *= s; z *= s;
			return *this;
		}

		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y) || std::isnan(z);
		}
		T x, y, z;
	};

	template <typename T, typename U>
	Point3<T> operator*(U f, const Point3<T>& p) {
		return p * f;
	}

	template <typename T>
	float Distance(const Point3<T> &p1, const Point3<T> &p2)
	{
		return (p1 - p2).Length();
	}

	template<typename T>
	float DistanceSquared(const Point3<T> &p1, const Point3<T> &p2)
	{
		return (p1 - p2).LengthSquared();
	}

	template<typename T>
	Point3<T> Lerp(float t, const Point3<T> &p0, const Point3<T> &p1)
	{
		return (1 - t) * p0 + t * p1;
	}

	template<typename T>
	Point3<T> Min(const Point3<T> &p1, const Point3<T> &p2)
	{
		return Point3<T>(
			std::min(p1.x, p2.x),
			std::min(p1.y, p2.y),
			std::min(p1.z, p2.z)
			);
	}

	template<typename T>
	Point3<T> Max(const Point3<T>& p1, const Point3<T>& p2)
	{
		return Point3<T>(
			std::max(p1.x, p2.x),
			std::max(p1.y, p2.y),
			std::max(p1.z, p2.z)
			);
	}

	template<typename T>
	Point3<T> Floor(const Point3<T> &p)
	{
		return (std::floor(p.x), std::floor(p.y), std::floor(p.z));
	}

	template<typename T>
	Point3<T> Ceil(const Point3<T>& p)
	{
		return (std::ceil(p.x), std::ceil(p.y), std::ceil(p.z));
	}

	template<typename T>
	Point3<T> Abs(const Point3<T>& p)
	{
		return (std::abs(p.x), std::abs(p.y), std::abs(p.z));
	}

	template<typename T>
	Point3<T> Permute(const Point3<T> &p, size_t x, size_t y, size_t z)
	{
		return Point3<T>(p[x], p[y], p[z]);
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const Point3<T>& v)
	{
		os << fmt::format("[ {}, {}, {} ]", v.x, v.y, v.z);
		return os;
	}

	typedef Point2<float> Point2f;
	typedef Point2<int> Point2i;
	typedef Point3<float> Point3f;
	typedef Point3<int> Point3i;

	template <typename T>
	class Bounds2
	{
	public:
		Bounds2(const Point2<T>& p1, const Point2<T>& p2);
		Vector2<T> Diagonal() const;
		Point2<T> pMin, pMax;
		bool operator==(const Bounds2<T>& b) const {
			return b.pMin == pMin && b.pMax == pMax;
		}
		bool operator!=(const Bounds2<T>& b) const {
			return b.pMin != pMin || b.pMax != pMax;
		}
	};

	template <typename T>
	class Normal2{};
	template <typename T>
	class Normal3
	{
	public:
		T x, y, z;
	};

	typedef Normal3<float> Normal3f;


	template <typename T>
	inline T Dot(const Vector3<T>& v1, const Vector3<T>& v2) {
		//DCHECK(!v1.HasNaNs() && !v2.HasNaNs());
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	template <typename T>
	inline T AbsDot(const Vector3<T>& v1, const Vector3<T>& v2) {
		//DCHECK(!v1.HasNaNs() && !v2.HasNaNs());
		return std::abs(Dot(v1, v2));
	}
	template <typename T>
	inline T AbsDot(const Vector3<T>& v1, const Normal3<T>& n2) {
		//DCHECK(!v1.HasNaNs() && !n2.HasNaNs());
		return std::abs(v1.x * n2.x + v1.y * n2.y + v1.z * n2.z);
	}

	template <typename T>
	inline T AbsDot(const Normal3<T>& n1, const Normal3<T>& n2) {
		//DCHECK(!n1.HasNaNs() && !n2.HasNaNs());
		return std::abs(n1.x * n2.x + n1.y * n2.y + n1.z * n2.z);
	}

	class Ray {};
	class RayDifferential : public Ray
	{
	public:
		void ScaleDifferentials(float s);
	};
	template <typename T>
	class Bounds3 {};

	typedef Bounds2<float> Bounds2f;
	typedef Bounds2<int> Bounds2i;
	typedef Bounds3<float> Bounds3f;
	typedef Bounds3<int> Bounds3i;

	class Bounds2iIterator : public std::forward_iterator_tag
	{
	public:
		Bounds2iIterator operator++();
		Bounds2iIterator operator++(int);
		bool operator==(const Bounds2iIterator& bi) const;
		bool operator!= (const Bounds2iIterator& bi) const;
		Point2i operator*() const;
	};
	inline Bounds2iIterator begin(const Bounds2i& b);
	inline Bounds2iIterator end(const Bounds2i& e);
}

#endif