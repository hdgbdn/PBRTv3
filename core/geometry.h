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
		Vector2() : x(0), y(0)
		{
		}

		Vector2(T xx, T yy) : x(xx), y(yy)
		{
			assert(!HasNaNs());
		}

		Vector2(const Normal2<T>& n) : x(n.x), y(n.y)
		{
			assert(!n.HasNaNs());
		}

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
			x += rhs.x;
			y += rhs.y;;
			return *this;
		}

		Vector2<T> operator-(const Vector2<T>& rhs)
		{
			return Vector2<T>(x - rhs.x, y - rhs.y);
		}

		Vector2<T>& operator-=(const Vector2<T>& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		Vector2<T> operator*(T s) const
		{
			return Vector2<T>(s * x, s * y);
		}

		Vector2<T>& operator*=(T s)
		{
			x *= s;
			y *= s;
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
			x *= inv;
			y *= inv;
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
		Vector3() : x(0), y(0), z(0)
		{
		}

		Vector3(T xx, T yy, T zz) : x(xx), y(yy), z(zz)
		{
			assert(!HasNaNs());
		}

		Vector3(const Normal3<T>& n) : x(n.x), y(n.y), z(n.z)
		{
			assert(!n.HasNaNs());
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
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		Vector3<T> operator-() const
		{
			return Vector3<T>(-x, -y, -z);
		}

		Vector3<T> operator-(const Vector3<T>& rhs)
		{
			return Vector3<T>(x - rhs.x, y - rhs.y, z - rhs.z);
		}

		Vector3<T>& operator-=(const Vector3<T>& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		Vector3<T> operator*(T s) const
		{
			return Vector3<T>(s * x, s * y, s * z);
		}

		Vector3<T>& operator*=(T s)
		{
			x *= s;
			y *= s;
			z *= s;
			return *this;
		}

		Vector3<T> operator/(T f) const
		{
			assert(f != 0);
			float inv = static_cast<float>(1 / f);
			return Vector3(x * inv, y * inv, z * inv);
		}

		Vector3<T>& operator/=(T f)
		{
			assert(f != 0);
			float inv = static_cast<float>(1 / f);
			x *= inv;
			y *= inv;
			z *= inv;
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

	template <typename T, typename U>
	Vector3<T>& operator*(U s, const Vector3<T>& v)
	{
		return s * v;
	}

	template <typename T>
	Vector3<T> Abs(const Vector3<T>& v)
	{
		return Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
	}

	template <typename T>
	Vector3<T> Cross(const Vector3<T>& lhs, const Vector3<T>& rhs)
	{
		double v1x = lhs.x, v1y = lhs.y, v1z = lhs.z;
		double v2x = rhs.x, v2y = rhs.y, v2z = rhs.z;
		return Vector3<T>(
			(v1y * v2z) - (v1z * v2y),
			(v1z * v2x) - (v1x * v2z),
			(v1x * v2y) - (v1y * v2x)
		);
	}

	template <typename T>
	Vector3<T> Cross(const Vector3<T>& lhs, const Normal3<T>& rhs)
	{
		double v1x = lhs.x, v1y = lhs.y, v1z = lhs.z;
		double v2x = rhs.x, v2y = rhs.y, v2z = rhs.z;
		return Vector3<T>(
			(v1y * v2z) - (v1z * v2y),
			(v1z * v2x) - (v1x * v2z),
			(v1x * v2y) - (v1y * v2x)
		);
	}

	template <typename T>
	Vector3<T> Cross(const Normal3<T>& lhs, const Vector3<T>& rhs)
	{
		double v1x = lhs.x, v1y = lhs.y, v1z = lhs.z;
		double v2x = rhs.x, v2y = rhs.y, v2z = rhs.z;
		return Vector3<T>(
			(v1y * v2z) - (v1z * v2y),
			(v1z * v2x) - (v1x * v2z),
			(v1x * v2y) - (v1y * v2x)
		);
	}

	template <typename T>
	Vector3<T> Normalize(const Vector3<T>& v)
	{
		return v / v.Length();
	}

	template <typename T>
	Normal3<T> Normalize(const Normal3<T>& v)
	{
		return v / v.Length();
	}

	template <typename T>
	T MinComponent(const Vector3<T>& v)
	{
		return std::min(v.x, std::min(v.y, v.z));
	}

	template <typename T>
	T MaxComponent(const Vector3<T>& v)
	{
		return std::max(v.x, std::max(v.y, v.z));
	}

	template <typename T>
	size_t MaxDimension(const Vector3<T>& v)
	{
		return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) : ((v.y > v.z) ? 1 : 2);
	}

	template <typename T>
	size_t MinDimension(const Vector3<T>& v)
	{
		return (v.x < v.y) ? ((v.x < v.z) ? 0 : 2) : ((v.y < v.z) ? 1 : 2);
	}

	template <typename T>
	Vector3<T> Min(const Vector3<T>& lhs, const Vector3<T>& rhs)
	{
		return Vector3<T>(
			std::min(lhs.x, rhs.x),
			std::min(lhs.y, rhs.y),
			std::min(lhs.z, rhs.z)
		);
	}

	template <typename T>
	Vector3<T> Max(const Vector3<T>& lhs, const Vector3<T>& rhs)
	{
		return Vector3<T>(
			std::max(lhs.x, rhs.x),
			std::max(lhs.y, rhs.y),
			std::max(lhs.z, rhs.z)
		);
	}

	template <typename T>
	Vector3<T> Permute(const Vector3<T>& v, size_t x, size_t y, size_t z)
	{
		return Vector3<T>(v[x], v[y], v[z]);
	}

	template <typename T>
	void CoordinateSystem(const Vector3<T>& v1, Vector3<T>* v2, Vector3<T>* v3)
	{
		if (std::abs(v1.x) > std::abs(v1.y))
			*v2 = Vector3<T>(-v1.z, 0, v1.x) /
				std::sqrt(v1.x * v1.x + v1.z * v1.z);
		else
			*v2 = Vector3<T>(-v1.z, 0, v1.x) /
				std::sqrt(v1.x * v1.x + v1.z * v1.z);
		*v3 = Cross(v1, v2);
	}

	template <typename T>
	T Dot(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		//DCHECK(!v1.HasNaNs() && !v2.HasNaNs());
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	template <typename T>
	T AbsDot(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		//DCHECK(!v1.HasNaNs() && !v2.HasNaNs());
		return std::abs(Dot(v1, v2));
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& os, const Vector3<T>& v)
	{
		os << fmt::format("[ {}, {}, {} ]", v.x, v.y, v.z);
		return os;
	}

	using Vector2f = Vector2<float>;
	using Vector2i = Vector2<int>;
	using Vector3f = Vector3<float>;
	using Vector3i = Vector3<int>;

	template <typename T>
	class Point2
	{
	public:
		Point2() = default;

		Point2(T xx, T yy) : x(xx), y(yy)
		{
			assert(!HasNaNs());
		}

		Point2(const Point3<T>& p) : x(p.x), y(p.y)
		{
			assert(!HasNaNs());
		}

		template <typename U>
		explicit Point2(const Point2<U>& p)
		{
			x = (T)p.x;
			y = (T)p.y;
			assert(!HasNaNs());
		}

		template <typename U>
		explicit Point2(const Vector2<U>& p)
		{
			x = (T)p.x;
			y = (T)p.y;
			assert(!HasNaNs());
		}

		template <typename U>
		explicit operator Vector2<U>() const
		{
			return Vector2<U>(x, y);
		}

		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y);
		}

		Point2<T> operator+(const Vector2<T>& v) const
		{
			// DCHECK(!v.HasNaNs());
			return Point2<T>(x + v.x, y + v.y);
		}

		Point2<T>& operator+=(const Vector2<T>& v)
		{
			// DCHECK(!v.HasNaNs());
			x += v.x;
			y += v.y;
			return *this;
		}

		Vector2<T> operator-(const Point2<T>& p) const
		{
			// DCHECK(!p.HasNaNs());
			return Vector2<T>(x - p.x, y - p.y);
		}

		Point2<T> operator-(const Vector2<T>& v) const
		{
			// DCHECK(!v.HasNaNs());
			return Point2<T>(x - v.x, y - v.y);
		}

		T operator[](size_t i) const
		{
			assert(i <= 2);
			if (i == 0) return x;
			return y;
		}

		T x, y;
	};

	template <typename T>
	Vector2<T> operator-(const Point2<T>& lhs, const Point2<T>& rhs)
	{
		return Vector2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
	}

	template <typename T>
	Point2<T> operator+(const Point2<T>& lhs, const Point2<T>& rhs)
	{
		return Point2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
	}

	template <typename T, typename U>
	Point2<T> operator*(U f, const Point2<T>& rhs)
	{
		return Point2<T>(f * rhs.x, f * rhs.y);
	}

	template <typename T>
	Point2<T> Floor(const Point2<T>& p)
	{
		return (std::floor(p.x), std::floor(p.y));
	}

	template <typename T>
	Point2<T> Ceil(const Point2<T>& p)
	{
		return (std::ceil(p.x), std::ceil(p.y));
	}

	template <typename T>
	class Point3
	{
	public:
		Point3() = default;

		Point3(T v) : x(v), y(v), z(v)
		{
			assert(!HasNaNs());
		}

		Point3(T xx, T yy, T zz) : x(xx), y(yy), z(zz)
		{
			assert(!HasNaNs());
		}

		template <typename U>
		explicit Point3(const Point3<U>& p)
			: x(static_cast<T>(p.x)), y(static_cast<T>(p.y)), z(static_cast<T>(p.z))
		{
			assert(!HasNaNs());
		}

		template <typename U>
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

		Point3<T>& operator=(const Point3<T>& p)
		{
			x = p.x;
			y = p.y;
			z = p.z;
			return *this;
		}

		Point3<T> operator+(const Vector3<T>& v) const
		{
			return Point3<T>(x + v.x, y + v.y, z + v.z);
		}

		Point3<T>& operator+=(const Vector3<T>& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		Vector3<T> operator-(const Point3<T>& p) const
		{
			return Vector3<T>(x - p.x, y - p.y, z - p.z);
		}

		Point3<T> operator-(const Vector3<T>& v) const
		{
			return Point3<T>(x - v.x, y - v.y, z - v.z);
		}

		Point3<T>& operator-=(const Vector3<T>& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}

		Point3<T> operator+(const Point3<T>& rhs)
		{
			return Point3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
		}

		template <typename U>
		Point3<T> operator*(U s) const
		{
			return Point3<T>(s * x, s * y, s * z);
		}

		template <typename U>
		Point3<T>& operator*=(U s)
		{
			x *= s;
			y *= s;
			z *= s;
			return *this;
		}

		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y) || std::isnan(z);
		}

		T x, y, z;
	};

	template <typename T, typename U>
	Point3<T> operator*(U f, const Point3<T>& p)
	{
		return p * f;
	}

	template <typename T>
	float Distance(const Point3<T>& p1, const Point3<T>& p2)
	{
		return (p1 - p2).Length();
	}

	template <typename T>
	float DistanceSquared(const Point3<T>& p1, const Point3<T>& p2)
	{
		return (p1 - p2).LengthSquared();
	}

	template <typename T>
	Point3<T> Lerp(float t, const Point3<T>& p0, const Point3<T>& p1)
	{
		return (1 - t) * p0 + t * p1;
	}

	template <typename T>
	Point3<T> Min(const Point3<T>& p1, const Point3<T>& p2)
	{
		return Point3<T>(
			std::min(p1.x, p2.x),
			std::min(p1.y, p2.y),
			std::min(p1.z, p2.z)
		);
	}

	template <typename T>
	Point3<T> Max(const Point3<T>& p1, const Point3<T>& p2)
	{
		return Point3<T>(
			std::max(p1.x, p2.x),
			std::max(p1.y, p2.y),
			std::max(p1.z, p2.z)
		);
	}

	template <typename T>
	Point3<T> Floor(const Point3<T>& p)
	{
		return (std::floor(p.x), std::floor(p.y), std::floor(p.z));
	}

	template <typename T>
	Point3<T> Ceil(const Point3<T>& p)
	{
		return (std::ceil(p.x), std::ceil(p.y), std::ceil(p.z));
	}

	template <typename T>
	Point3<T> Abs(const Point3<T>& p)
	{
		return (std::abs(p.x), std::abs(p.y), std::abs(p.z));
	}

	template <typename T>
	Point3<T> Permute(const Point3<T>& p, size_t x, size_t y, size_t z)
	{
		return Point3<T>(p[x], p[y], p[z]);
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& os, const Point3<T>& v)
	{
		os << fmt::format("[ {}, {}, {} ]", v.x, v.y, v.z);
		return os;
	}

	using Point2f = Point2<float>;
	using Point2i = Point2<int>;
	using Point3f = Point3<float>;
	using Point3i = Point3<int>;

	template <typename T>
	class Normal2
	{
	};

	template <typename T>
	class Normal3
	{
	public:
		Normal3<T>(): x(0), y(0), z(0)
		{
		}

		Normal3<T>(T xx, T yy, T zz) : x(xx), y(yy), z(zz)
		{
			assert(!HasNaNs());
		}

		explicit Normal3<T>(const Vector3<T>& v) : x(v.x), y(v.y), z(v.z)
		{
			assert(!v.HasNaNs());
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

		Normal3<T> operator+(const Normal3<T>& n) const
		{
			return Normal3<T>(x + n.x, y + n.y, z + n.z);
		}

		Normal3<T>& operator+=(const Normal3<T>& n)
		{
			x += n.x;
			y += n.y;
			z += n.z;
			return *this;
		}

		Normal3<T> operator-() const
		{
			return Normal3<T>(-x, -y, -z);
		}

		Normal3<T> operator-(const Normal3<T>& n) const
		{
			return Normal3<T>(x - n.x, y - n.y, z - n.z);
		}

		Normal3<T>& operator-=(const Normal3<T>& n) const
		{
			x -= n.x;
			y -= n.y;
			z -= n.z;
			return *this;
		}

		template <typename U>
		Normal3<T> operator*(U f) const
		{
			return Normal3<T>(x * f, y * f, z * f);
		}

		template <typename U>
		Normal3<T>& operator*=(U f)
		{
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}

		template <typename U>
		Normal3<T> operator/(U f) const
		{
			return Normal3<T>(x / f, y / f, z / f);
		}

		template <typename U>
		Normal3<T>& operator/=(U f)
		{
			x /= f;
			y /= f;
			z /= f;
			return *this;
		}

		bool operator==(const Normal3<T>& rhs) const
		{
			return (rhs.x == x) || (rhs.y == y) || (rhs.z == z);
		}

		bool operator!=(const Normal3<T>& rhs) const
		{
			return (rhs.x != x) || (rhs.y != y) || (rhs.z != z);
		}

		float LengthSquared() const
		{
			return x * x + y * y + z * z;
		}

		float Length() const
		{
			return std::sqrt(LengthSquared());
		}

		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y) || std::isnan(z);
		}

		T x, y, z;
	};

	template <typename T, typename U>
	Normal3<T> operator*(U f, const Normal3<T>& n)
	{
		return Normal3<T>(f * n.x, f * n.y, f * n.z);
	}

	template <typename T>
	T Dot(const Normal3<T>& n1, const Normal3<T>& n2)
	{
		//DCHECK(!n1.HasNaNs() && !n2.HasNaNs());
		return n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
	}

	template <typename T>
	T AbsDot(const Normal3<T>& n1, const Normal3<T>& n2)
	{
		//DCHECK(!n1.HasNaNs() && !n2.HasNaNs());
		return std::abs(n1.x * n2.x + n1.y * n2.y + n1.z * n2.z);
	}

	template <typename T>
	T Dot(const Vector3<T>& v1, const Normal3<T>& n2)
	{
		//DCHECK(!v1.HasNaNs() && !n2.HasNaNs());
		return v1.x * n2.x + v1.y * n2.y + v1.z * n2.z;
	}

	template <typename T>
	T AbsDot(const Vector3<T>& v1, const Normal3<T>& n2)
	{
		//DCHECK(!v1.HasNaNs() && !n2.HasNaNs());
		return std::abs(v1.x * n2.x + v1.y * n2.y + v1.z * n2.z);
	}

	template <typename T>
	T Dot(const Normal3<T>& n1, const Vector3<T>& v2)
	{
		//DCHECK(!v1.HasNaNs() && !n2.HasNaNs());
		return n1.x * v2.x + n1.y * v2.y + n1.z * v2.z;
	}

	template <typename T>
	T AbsDot(const Normal3<T>& n1, const Vector3<T>& v2)
	{
		//DCHECK(!v1.HasNaNs() && !n2.HasNaNs());
		return std::abs(n1.x * v2.x + n1.y * v2.y + n1.z * v2.z);
	}

	template <typename T>
	Normal3<T> Faceforward(const Normal3<T>& n, const Vector3<T>& v)
	{
		return (Dot(n, v) < 0.f) ? -n : n;
	}

	template <typename T>
	Normal3<T> Faceforward(const Vector3<T>& v, const Normal3<T>& n)
	{
		return (Dot(v, n) < 0.f) ? -v : v;
	}

	template <typename T>
	Normal3<T> Faceforward(const Normal3<T>& n1, const Normal3<T>& n2)
	{
		return (Dot(n1, n2) < 0.f) ? -n1 : n1;
	}

	template <typename T>
	Normal3<T> Faceforward(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		return (Dot(v1, v2) < 0.f) ? -v1 : v1;
	}


	using Normal3f = Normal3<float>;

	class Ray
	{
	public:
		Ray() : tMax(Infinity), time(0.f), medium(nullptr)
		{
		}

		Ray(const Point3f& o, const Vector3f& d, float tMax = Infinity,
		    float time = 0.f, const Medium* medium = nullptr)
			: o(o), d(d), tMax(tMax), time(time), medium(medium)
		{
		}

		Point3f operator()(float t) const
		{
			return o + t * d;
		}

		Point3f o;
		Vector3f d;
		mutable float tMax;
		float time;
		const Medium* medium;
	};

	class RayDifferential : public Ray
	{
	public:
		RayDifferential()
			: Ray(), hasDifferentials(false),
			  rxOrigin(), ryOrigin(), rxDirection(), ryDirection()
		{
		}

		RayDifferential(const Ray& r) : Ray(r), hasDifferentials(false),
		                                rxOrigin(), ryOrigin(), rxDirection(), ryDirection()
		{
		}

		RayDifferential(const Point3f& o, const Vector3f& d, float tMax = Infinity,
		                float time = 0.f, const Medium* medium = nullptr)
			: Ray(o, d, tMax, time, medium), hasDifferentials(false),
			  rxOrigin(), ryOrigin(), rxDirection(), ryDirection()
		{
		}

		void ScaleDifferentials(float s)
		{
			rxOrigin = o + (rxOrigin - o) * s;
			ryOrigin = o + (ryOrigin - o) * s;
			rxDirection = d + (rxDirection - d) * s;
			ryDirection = d + (ryDirection - d) * s;
		}

		bool hasDifferentials;
		Point3f rxOrigin, ryOrigin;
		Vector3f rxDirection, ryDirection;
	};

	template <typename T>
	class Bounds2
	{
	public:
		const T minNum = std::numeric_limits<T>::lowest();
		const T maxNum = std::numeric_limits<T>::max();

		Bounds2() : pMin(maxNum), pMax(minNum)
		{
		}

		explicit Bounds2(const Point2<T>& p) : pMin(p), pMax(p)
		{
		}

		Bounds2(const Point2<T>& p1, const Point2<T>& p2):
			pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
			pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y))
		{
		}
		template <typename U>
		explicit Bounds2(const Bounds2<U>& p)
		{
			pMin = (T)p.pMin;
			pMax = (T)p.pMax;
		}

		Vector2<T> Diagonal() const;
		Point2<T> pMin, pMax;

		Bounds2& operator=(const Bounds2& rhs)
		{
			pMin = rhs.pMin;
			pMax = rhs.pMax;
			return *this;
		}

		bool operator==(const Bounds2<T>& b) const
		{
			return b.pMin == pMin && b.pMax == pMax;
		}

		bool operator!=(const Bounds2<T>& b) const
		{
			return b.pMin != pMin || b.pMax != pMax;
		}

		T Area() const
		{
			Vector2<T> d = pMax - pMin;
			return (d.x * d.y);
		}
	};

	using Bounds2f = Bounds2<float>;
	using Bounds2i = Bounds2<int>;

	template <typename T>
	Bounds2<T> Intersect(const Bounds2<T>& b1, const Bounds2<T>& b2) {
		return Bounds2<T>(Point2<T>(std::max(b1.pMin.x, b2.pMin.x),
			std::max(b1.pMin.y, b2.pMin.y)),
			Point2<T>(std::min(b1.pMax.x, b2.pMax.x),
				std::min(b1.pMax.y, b2.pMax.y)));
	}

	template <typename T>
	bool InsideExclusive(const Point2<T>& p, const Bounds2<T>& b)
	{
		return (
			p.x >= b.pMin.x && p.x < b.pMax.x&&
			p.y >= b.pMin.y && p.y < b.pMax.y);
	}

	class Bounds2iIterator : public std::forward_iterator_tag
	{
	public:
		Bounds2iIterator operator++();
		Bounds2iIterator operator++(int);
		bool operator==(const Bounds2iIterator& bi) const;
		bool operator!=(const Bounds2iIterator& bi) const;
		Point2i operator*() const;
	};

	inline Bounds2iIterator begin(const Bounds2i& b);
	inline Bounds2iIterator end(const Bounds2i& e);

	template <typename T>
	class Bounds3
	{
	public:
		const T minNum = std::numeric_limits<T>::lowest();
		const T maxNum = std::numeric_limits<T>::max();

		Bounds3() : pMin(maxNum), pMax(minNum)
		{
		}

		explicit Bounds3(const Point3<T>& p) : pMin(p), pMax(p)
		{
		}

		Bounds3(const Point3<T>& p1, const Point3<T>& p2) :
			pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)),
			pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z))
		{
		}

		bool IntersectP(const Ray& ray, float* hitt0, float* hitt1) const;
		bool IntersectP(const Ray& ray, const Vector3f& invDir,
		                const int dirIsNeg[3]) const;

		const Point3<T>& operator[](size_t i) const
		{
			assert(i <= 1);
			if (i == 0) return pMin;
			return pMax;
		}

		Point3<T>& operator[](size_t i)
		{
			assert(i <= 1);
			if (i == 0) return pMin;
			return pMax;
		}

		Bounds3& operator=(const Bounds3& rhs)
		{
			pMin = rhs.pMin;
			pMax = rhs.pMax;
			return *this;
		}

		Point3<T> Corner(int corner) const
		{
			return Point3<T>(
				(*this)[corner & 1].x,
				(*this)[(corner & 2) ? 1 : 0].y,
				(*this)[(corner & 4) ? 1 : 0].z
			);
		}

		Vector3<T> Diagonal() const { return pMax - pMin; }

		T SurfaceArea() const
		{
			Vector3<T> d = Diagonal();
			return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
		}

		T Volume() const
		{
			Vector3<T> d = Diagonal();
			return d.x * d.y * d.z;
		}

		int MaximumExtent() const
		{
			Vector3<T> d = Diagonal();
			if (d.x > d.y && d.x > d.z) return 0;
			else if (d.y > d.z) return 1;
			else return 2;
		}

		Point3<T> Lerp(const Point3f& t) const
		{
			return Point3<T>(
				Lerp(t.x, pMin.x, pMax.x),
				Lerp(t.x, pMin.y, pMax.y),
				Lerp(t.x, pMin.z, pMax.z));
		}

		Vector3<T> Offset(const Point3<T>& p) const
		{
			Vector3<T> o = p - pMin;
			if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
			if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
			if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
			return o;
		}

		Bounds3& Union(const Bounds3<T>& b)
		{
			pMin = Point3<T>(
				std::min(pMin.x, b.pMin.x),
				std::min(pMin.y, b.pMin.y),
				std::min(pMin.z, b.pMin.z)
			);
			pMax = Point3<T>(
				std::max(pMax.x, b.pMax.x),
				std::max(pMax.y, b.pMax.y),
				std::max(pMax.z, b.pMax.z)
			);
			return *this;
		}

		Bounds3& Union(const Point3<T>& p)
		{
			pMin = Point3<T>(
				std::min(pMin.x, p.x),
				std::min(pMin.y, p.y),
				std::min(pMin.z, p.z)
			);
			pMax = Point3<T>(
				std::max(pMax.x, p.x),
				std::max(pMax.y, p.y),
				std::max(pMax.z, p.z)
			);
			return *this;
		}

		void BoundingSphere(Point3<T>* center, float* radius) const
		{
			*center = (pMin + pMax) / 2;
			*radius = Inside(*center, *this) ? Distance(*center, pMax) : 0;
		}

		Point3<T> pMin, pMax;
	};

	template <typename T>
	bool Bounds3<T>::IntersectP(const Ray& ray, float* hitt0, float* hitt1) const
	{
		float t0 = 0, t1 = ray.tMax;
		for (int i = 0; i < 3; ++i)
		{
			float invRayDir = 1 / ray.d[i];
			float tNear = (pMin.x - ray.o[i]) * invRayDir;
			float tFar = (pMax.x - ray.o[i]) * invRayDir;
			if (tNear > tFar) std::swap(tNear, tFar);
			// TODO Update tFar to ensure robust ray–bounds intersection
			t0 = tNear > t0 ? tNear : t0;
			t1 = tFar < t1 ? tFar : t1;
			if (t0 > t1) return false;
		}
		if (nullptr != hitt0) *hitt0 = t0;
		if (nullptr != hitt1) *hitt1 = t1;
		return true;
	}

	template <typename T>
	bool Bounds3<T>::IntersectP(const Ray& ray, const Vector3f& invDir, const int dirIsNeg[3]) const
	{
		const Bounds3<float>& bounds = *this;
		float tMin = (bounds[dirIsNeg[0]].x - ray.o.x) * invDir.x;
		float tMax = (bounds[1 - dirIsNeg[0]].x - ray.o.x) * invDir.x;
		float tyMin = (bounds[dirIsNeg[1]].y - ray.o.y) * invDir.y;
		float tyMax = (bounds[1 - dirIsNeg[1]].y - ray.o.y) * invDir.y;
		// TODO Update tMax and tyMax to ensure robust bounds intersectio
		if (tMin > tyMax || tyMin > tMax) return false;
		if (tyMin > tMin) tMin = tyMin;
		if (tyMax < tMax) tMax = tyMax;

		float tzMin = (bounds[dirIsNeg[2]].z - ray.o.z) * invDir.z;
		float tzMax = (bounds[1 - dirIsNeg[2]].z - ray.o.z) * invDir.z;
		if (tzMin > tMax || tzMax < tMin) return false;
		if (tzMin > tMin) tMin = tzMin;
		if (tzMax < tMax) tMax = tzMax;
		return (tMin < ray.tMax) && (tMax > 0);
	}

	template <typename T>
	Bounds3<T> Union(const Bounds3<T>& b, const Point3<T>& p)
	{
		return Bounds3<T>(
			Point3<T>(
				std::min(b.pMin.x, p.x),
				std::min(b.pMin.y, p.y),
				std::min(b.pMin.z, p.z)
			),
			Point3<T>(
				std::max(b.pMax.x, p.x),
				std::max(b.pMax.y, p.y),
				std::max(b.pMax.z, p.z)
			)
		);
	}

	template <typename T> Bounds3<T>
	Intersect(const Bounds3<T>& b1, const Bounds3<T>& b2) {
		return Bounds3<T>(Point3<T>(std::max(b1.pMin.x, b2.pMin.x),
			std::max(b1.pMin.y, b2.pMin.y),
			std::max(b1.pMin.z, b2.pMin.z)),
			Point3<T>(std::min(b1.pMax.x, b2.pMax.x),
				std::min(b1.pMax.y, b2.pMax.y),
				std::min(b1.pMax.z, b2.pMax.z)));
	}

	template <typename T>
	Bounds3<T> Union(const Bounds3<T>& b1, const Bounds3<T>& b2)
	{
		return Bounds3<T>(
			Point3<T>(
				std::min(b1.pMin.x, b2.pMin.x),
				std::min(b1.pMin.y, b2.pMin.y),
				std::min(b1.pMin.z, b2.pMin.z)
			),
			Point3<T>(
				std::max(b1.pMax.x, b2.pMax.x),
				std::max(b1.pMax.y, b2.pMax.y),
				std::max(b1.pMax.z, b2.pMax.z)
			)
		);
	}

	template <typename T>
	bool Overlaps(const Bounds3<T>& b1, const Bounds3<T>& b2)
	{
		bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
		bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
		bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
		return (x && y && z);
	}

	template <typename T>
	bool Inside(const Point3<T>& p, const Bounds3<T>& b)
	{
		return (
			p.x >= b.pMin.x && p.x <= b.pMax.x &&
			p.y >= b.pMin.y && p.y <= b.pMax.y &&
			p.z >= b.pMax.z && p.z <= b.pMax.z);
	}

	template <typename T>
	bool InsideExclusive(const Point3<T>& p, const Bounds3<T>& b)
	{
		return (
			p.x >= b.pMin.x && p.x < b.pMax.x &&
			p.y >= b.pMin.y && p.y < b.pMax.y &&
			p.z >= b.pMax.z && p.z < b.pMax.z);
	}

	template <typename T, typename U>
	Bounds3<T> Expand(const Bounds3<T>& b, U delta)
	{
		return Bounds3<T>(
			b.pMin - Vector3<T>(delta),
			b.pMax + Vector3<T>(delta));
	}

	using Bounds3f = Bounds3<float>;
	using Bounds3i = Bounds3<int>;

	inline Vector3f SphericalDirection(float sinTheta,
	                                   float cosTheta, float phi)
	{
		return Vector3f(sinTheta * std::cos(phi),
		                sinTheta * std::sin(phi),
		                cosTheta);
	}

	inline Vector3f SphericalDirection(float sinTheta, float cosTheta,
	                                   float phi, const Vector3f& x, const Vector3f& y,
	                                   const Vector3f& z)
	{
		return sinTheta * std::cos(phi) * x +
			sinTheta * std::sin(phi) * y + cosTheta * z;
	}

	inline float SphericalTheta(const Vector3f& v)
	{
		return std::acos(Clamp(v.z, -1, 1));
	}

	inline float SphericalPhi(const Vector3f& v)
	{
		float p = std::atan2(v.y, v.x);
		return p < 0 ? (p + 2 * Pi) : p;
	}
}

#endif
