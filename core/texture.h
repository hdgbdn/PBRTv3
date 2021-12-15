#ifndef PBRT_CORE_TEXTURE_H
#define PBRT_CORE_TEXTURE_H

#include "pbrt.h"
#include "interaction.h"
#include "transformation.h"

namespace pbrt
{
    class TextureMapping2D
    {
    public:
        ~TextureMapping2D();
        virtual Point2f Map(const SurfaceInteraction& si, Vector2f* dstdx,
            Vector2f* dstdy) const = 0;
    };

    class UVMapping2D : public TextureMapping2D
    {
    public:
	    UVMapping2D(float su, float sv, float du, float dv)
		    : su(su),
		      sv(sv),
		      du(du),
		      dv(dv)
	    {
	    }

	    Point2f Map(const SurfaceInteraction& si, Vector2f* dstdx, Vector2f* dstdy) const override
	    {
            *dstdx = Vector2f(su * si.dudx, sv * si.dvdx);
            *dstdy = Vector2f(su * si.dudy, sv * si.dvdy);
            return { su * si.uv[0] + du,
            sv * si.uv[1] + dv };
	    }
    private:
        const float su, sv, du, dv;
    };

    class SphericalMapping2D : public TextureMapping2D
    {
    public:
        SphericalMapping2D(const Transform& WorldToTexture) : WorldToTexture(WorldToTexture) { }
        Point2f Map(const SurfaceInteraction& si, Vector2f* dstdx, Vector2f* dstdy) const override
        {
            Point2f st = sphere(si.p);
            const float delta = .1f;
            Point2f stDeltaX = sphere(si.p + delta * si.dpdx);
            *dstdx = (stDeltaX - st) / delta;
            Point2f stDeltaY = sphere(si.p + delta * si.dpdy);
            *dstdy = (stDeltaY - st) / delta;
            if ((*dstdx)[1] > .5)        (*dstdx)[1] = 1 - (*dstdx)[1];
            else if ((*dstdx)[1] < -.5f) (*dstdx)[1] = -((*dstdx)[1] + 1);
            if ((*dstdy)[1] > .5)        (*dstdy)[1] = 1 - (*dstdy)[1];
            else if ((*dstdy)[1] < -.5f) (*dstdy)[1] = -((*dstdy)[1] + 1);
            return st;
        }
    private:
        Point2f sphere(const Point3f& p) const
        {
            Vector3f vec = Normalize(WorldToTexture(p) - Point3f(0, 0, 0));
            float theta = SphericalTheta(vec), phi = SphericalPhi(vec);
            return Point2f(theta * InvPi, phi * Inv2Pi);
        }
        const Transform WorldToTexture;
    };

    class CylindricalMapping2D : public TextureMapping2D
    {
    public:
        CylindricalMapping2D(const Transform& WorldToTexture) : WorldToTexture(WorldToTexture) { }
        Point2f Map(const SurfaceInteraction& si, Vector2f* dstdx, Vector2f* dstdy) const override
        {
            Point2f st = cylinder(si.p);
            const float delta = .01f;
            Point2f stDeltaX = cylinder(si.p + delta * si.dpdx);
            *dstdx = (stDeltaX - st) / delta;
            if ((*dstdx)[1] > .5) (*dstdx)[1] = 1.f - (*dstdx)[1];
            else if ((*dstdx)[1] < -.5f) (*dstdx)[1] = -((*dstdx)[1] + 1);
            Point2f stDeltaY = cylinder(si.p + delta * si.dpdy);
            *dstdy = (stDeltaY - st) / delta;
            if ((*dstdy)[1] > .5) (*dstdy)[1] = 1.f - (*dstdy)[1];
            else if ((*dstdy)[1] < -.5f) (*dstdy)[1] = -((*dstdy)[1] + 1);
        }
    private:
        Point2f cylinder(const Point3f& p) const
        {
            Vector3f vec = Normalize(WorldToTexture(p) - Point3f(0, 0, 0));
            return Point2f((Pi + std::atan2(vec.y, vec.x)) * Inv2Pi, vec.z);
        }
        const Transform WorldToTexture;
    };

    class PlanarMapping2D : public TextureMapping2D
    {
    public:
        PlanarMapping2D(const Vector3f& vs, const Vector3f& vt,
            float ds = 0, float dt = 0)
	            : vs(vs), vt(vt), ds(ds), dt(dt) { }
        Point2f Map(const SurfaceInteraction& si, Vector2f* dstdx, Vector2f* dstdy) const override
        {
            Vector3f vec(si.p);
            *dstdx = Vector2f(Dot(si.dpdx, vs), Dot(si.dpdx, vt));
            *dstdy = Vector2f(Dot(si.dpdy, vs), Dot(si.dpdy, vt));
            return { ds + Dot(vec, vs), dt + Dot(vec, vt) };
        }
    private:
        const Vector3f vs, vt;
        const float ds, dt;
    };

    class TextureMapping3D
    {
    public:
        virtual Point3f Map(const SurfaceInteraction& si, Vector3f* dpdx, Vector3f* dpdy) const = 0;

    };

    class TransformMapping3D : public TextureMapping3D
    {
    public:
        Point3f Map(const SurfaceInteraction& si, Vector3f* dpdx, Vector3f* dpdy) const override
        {
            *dpdx = WorldToTexture(si.dpdx);
            *dpdy = WorldToTexture(si.dpdy);
            return WorldToTexture(si.p);
        }
    private:
        const Transform WorldToTexture;
    };

    template <typename T>
    class Texture {
    public:
        virtual ~Texture() {}
        virtual T Evaluate(const SurfaceInteraction&) const = 0;
    };

    float Lanczos(float x, float tau) {
        x = std::abs(x);
        if (x < 1e-5f) return 1;
        if (x > 1.f) return 0;
        x *= Pi;
        float s = std::sin(x * tau) / (x * tau);
        float lanczos = std::sin(x) / x;
        return s * lanczos;
    }
}

#endif