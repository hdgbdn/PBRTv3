#ifndef PBRT_CORE_TEXTURE_H
#define PBRT_CORE_TEXTURE_H

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
	    UVMapping2D(float su, float sv, float du, float dv);

	    Point2f Map(const SurfaceInteraction& si, Vector2f* dstdx, Vector2f* dstdy) const override;
    private:
        const float su, sv, du, dv;
    };

    class SphericalMapping2D : public TextureMapping2D
    {
    public:
        SphericalMapping2D(const Transform& WorldToTexture);

        Point2f Map(const SurfaceInteraction& si, Vector2f* dstdx, Vector2f* dstdy) const override;
    private:
        Point2f sphere(const Point3f& p) const;
        const Transform WorldToTexture;
    };

    class CylindricalMapping2D : public TextureMapping2D
    {
    public:
        CylindricalMapping2D(const Transform& WorldToTexture);

        Point2f Map(const SurfaceInteraction& si, Vector2f* dstdx, Vector2f* dstdy) const override;
    private:
        Point2f cylinder(const Point3f& p) const;
        const Transform WorldToTexture;
    };

    class PlanarMapping2D : public TextureMapping2D
    {
    public:
        PlanarMapping2D(const Vector3f& vs, const Vector3f& vt,
            float ds = 0, float dt = 0);

        Point2f Map(const SurfaceInteraction& si, Vector2f* dstdx, Vector2f* dstdy) const override;
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
        Point3f Map(const SurfaceInteraction& si, Vector3f* dpdx, Vector3f* dpdy) const override;
    private:
        const Transform WorldToTexture;
    };

    template <typename T>
    class Texture {
    public:
        virtual ~Texture() {}
        virtual T Evaluate(const SurfaceInteraction&) const = 0;
    };

    float Lanczos(float x, float tau = 2);
}

#endif