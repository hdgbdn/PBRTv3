#include "triangle.h"

namespace pbrt
{
	TriangleMesh::TriangleMesh(const Transform& ObjectToWorld, int nTriangles, const int* vertexIndices, int nVertices, const Point3f* P, const Vector3f* S, const Normal3f* N, const Point2f* UV, const std::shared_ptr<Texture<float>>& alphaMask)
		: nTriangles(nTriangles), nVertices(nVertices),
		vertexIndices(vertexIndices, vertexIndices + 3 * nTriangles), alphaMask(alphaMask)
	{
		p.reset(new Point3f[nVertices]);
		for (int i = 0; i < nVertices; i++)
			p[i] = ObjectToWorld(P[i]);

        if (UV) {
            uv.reset(new Point2f[nVertices]);
            memcpy(uv.get(), UV, nVertices * sizeof(Point2f));
        }
        if (N) {
            n.reset(new Normal3f[nVertices]);
            for (int i = 0; i < nVertices; ++i) n[i] = ObjectToWorld(N[i]);
        }
        if (S) {
            s.reset(new Vector3f[nVertices]);
            for (int i = 0; i < nVertices; ++i) s[i] = ObjectToWorld(S[i]);
        }
	}

    Triangle::Triangle(const std::shared_ptr<Transform>& ObjectToWorld, const std::shared_ptr<Transform>& WorldToObject, bool reverseOrientation, const std::shared_ptr<TriangleMesh>& mesh, int triNumber)
	    : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
    mesh(mesh)
	{
        v = &mesh->vertexIndices[3 * triNumber];
    }

    std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(const std::shared_ptr<Transform>& o2w, const std::shared_ptr<Transform>& w2o, bool reverseOrientation, int nTriangles, const int* vertexIndices, int nVertices, const Point3f* p, const Vector3f* s, const Normal3f* n, const Point2f* uv, const std::shared_ptr<Texture<float>>& alphaTexture, const std::shared_ptr<Texture<float>>& shadowAlphaTexture, const int* faceIndices)
    {
        std::shared_ptr<TriangleMesh> mesh =
            std::make_shared<TriangleMesh>(*o2w, nTriangles, vertexIndices,
                nVertices, p, s, n, uv, alphaTexture);
        std::vector<std::shared_ptr<Shape>> tris;
        for (int i = 0; i < nTriangles; ++i)
            tris.push_back(std::make_shared<Triangle>(o2w, w2o, reverseOrientation,
                mesh, i));
        return tris;
    }

    Bounds3f Triangle::ObjectBound() const
    {
        const Point3f& p0 = mesh->p[v[0]];
        const Point3f& p1 = mesh->p[v[1]];
        const Point3f& p2 = mesh->p[v[2]];
        return Union(Bounds3f((*WorldToObject)(p0), (*WorldToObject)(p1)),
            p2);
    }

    Bounds3f Triangle::WorldBound() const
    {
        const Point3f& p0 = mesh->p[v[0]];
        const Point3f& p1 = mesh->p[v[1]];
        const Point3f& p2 = mesh->p[v[2]];
        return Union(Bounds3f(p0, p1), p2);
    }

    bool Triangle::Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const
    {
        const Point3f& p0 = mesh->p[v[0]];
        const Point3f& p1 = mesh->p[v[1]];
        const Point3f& p2 = mesh->p[v[2]];

        Point3f p0t = p0 - Vector3f(ray.o);
        Point3f p1t = p1 - Vector3f(ray.o);
        Point3f p2t = p2 - Vector3f(ray.o);

        int kz = MaxDimension(Abs(ray.d));
        int kx = kz + 1; if (kx == 3) kx = 0;
        int ky = kx + 1; if (ky == 3) ky = 0;
        Vector3f d = Permute(ray.d, kx, ky, kz);
        p0t = Permute(p0t, kx, ky, kz);
        p1t = Permute(p1t, kx, ky, kz);
        p2t = Permute(p2t, kx, ky, kz);

        float Sx = -d.x / d.z;
        float Sy = -d.y / d.z;
        float Sz = 1.f / d.z;
        p0t.x += Sx * p0t.z;
        p0t.y += Sy * p0t.z;
        p1t.x += Sx * p1t.z;
        p1t.y += Sy * p1t.z;
        p2t.x += Sx * p2t.z;
        p2t.y += Sy * p2t.z;
    }


}