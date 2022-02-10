#include "triangle.h"

#include <utility>
#include "core/texture.h"
#include "core/sampling.h"
#include "core/paramset.h"
#include "textures/constant.h"

namespace pbrt
{
	TriangleMesh::TriangleMesh(const Transform &ObjectToWorld, int nTriangles,
                               const int* vertexIndices, int nVertices, const Point3f* P,
                               const Vector3f* S, const Normal3f* N, const Point2f* UV,
                               std::shared_ptr<Texture<float>>  alphaMask,
                               std::shared_ptr<Texture<float>>  shadowAlphaMask,
                               const int *fIndices)
		: nTriangles(nTriangles), nVertices(nVertices),
		vertexIndices(vertexIndices, vertexIndices + 3 * nTriangles), alphaMask(std::move(alphaMask)),
        shadowAlphaMask(std::move(shadowAlphaMask))
	{
		p.reset(new Point3f[nVertices]);
		for (int i = 0; i < nVertices; i++)
			p[i] = (ObjectToWorld)(P[i]);

        if (UV) {
            uv.reset(new Point2f[nVertices]);
            memcpy(uv.get(), UV, nVertices * sizeof(Point2f));
        }
        if (N) {
            n.reset(new Normal3f[nVertices]);
            for (int i = 0; i < nVertices; ++i) n[i] = (ObjectToWorld)(N[i]);
        }
        if (S) {
            s.reset(new Vector3f[nVertices]);
            for (int i = 0; i < nVertices; ++i) s[i] = (ObjectToWorld)(S[i]);
        }

        if (fIndices)
            faceIndices = std::vector<int>(fIndices, fIndices + nTriangles);
	}

    Triangle::~Triangle()
    {
        // don't delete v! it is just a pointer to tri mesh, wo didn't allocate any memory for it!
        // delete v;
    }

    Triangle::Triangle(const Transform *ObjectToWorld, const Transform *WorldToObject, bool reverseOrientation, const std::shared_ptr<TriangleMesh>& mesh, int triNumber)
	    : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
    mesh(mesh)
	{
        v = &mesh->vertexIndices[3 * triNumber];
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

        float e0 = p0t.x * p1t.y - p0t.y * p1t.x;
        float e1 = p1t.x * p2t.y - p1t.y * p2t.x;
        float e2 = p2t.x * p0t.y - p2t.y * p0t.x;

        if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
            return false;
        float det = e0 + e1 + e2;
        if (det == 0) return false;

        p0t.z *= Sz;
        p1t.z *= Sz;
        p2t.z *= Sz;
        float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
        if (det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det))
            return false;
        else if (det > 0 && (tScaled <= 0 || tScaled > ray.tMax * det))
            return false;
        float invDet = 1 / det;
        float b0 = e0 * invDet;
        float b1 = e1 * invDet;
        float b2 = e2 * invDet;
        float t = tScaled * invDet;

        Point2f uv[3];
        Vector3f dpdu, dpdv;
        GetUVs(uv);
        Vector2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
        Vector3f dp02 = p0 - p2, dp12 = p1 - p2;
        float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
        if (determinant == 0)
        {
            CoordinateSystem(Normalize(Cross(p2 - p0, p1 - p0)), &dpdu, &dpdv);
        }
        else
        {
            float invdet = 1 / determinant;
            dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
            dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
        }
        Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
        Point2f uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];

        if(testAlphaTexture && mesh->alphaMask)
        {
            SurfaceInteraction isectLocal(pHit, Vector3f(0, 0, 0), uvHit,
                Vector3f(0, 0, 0), dpdu, dpdv, Normal3f(0, 0, 0),
                Normal3f(0, 0, 0), ray.time, this);
            if (mesh->alphaMask->Evaluate(isectLocal) == 0) return false;
        }
        Vector3f pError;
        *isect = SurfaceInteraction(pHit, pError, uvHit,
            -ray.d, dpdu, dpdv, Normal3f(0, 0, 0),
            Normal3f(0, 0, 0), ray.time, this);
        isect->n = isect->shading.n = Normal3f(Normalize(Cross(dp02, dp12)));
        if(mesh->n || mesh->s)
        {
            Normal3f ns;
            if (mesh->n) ns = Normalize(b0 * mesh->n[v[0]] +
                b1 * mesh->n[v[1]] + b2 * mesh->n[v[2]]);
            else ns = isect->n;
            Vector3f ss;
            if (mesh->s) ss = Normalize(b0 * mesh->s[v[0]] +
                b1 * mesh->s[v[1]] + b2 * mesh->s[v[2]]);
            else ss = Normalize(isect->dpdu);
            Vector3f ts = Cross(ss, ns);
            if(ts.LengthSquared() > 0.f)
            {
                ts = Normalize(Cross(ns, ss));
                ss = Cross(ts, ns);
            }
            else
            {
                CoordinateSystem(static_cast<Vector3f>(ns), &ss, &ts);
            }
        }
        if (mesh->n)
            isect->n = Faceforward(isect->n, isect->shading.n);
        else if (reverseOrientation ^ transformSwapsHandedness)
            isect->n = isect->shading.n = -isect->n;
        return true;
    }

    Interaction Triangle::Sample(const Point2f& u) const
    {
        Point2f b = UniformSampleTriangle(u);
        const Point3f& p0 = mesh->p[v[0]];
        const Point3f& p1 = mesh->p[v[1]];
        const Point3f& p2 = mesh->p[v[2]];
        Interaction it;
        it.p = b[0] * p0 + b[1] * p1 + (1 - b[0] - b[1]) * p2;
        if (mesh->n)
            it.n = Normalize(b[0] * mesh->n[v[0]] +
                b[1] * mesh->n[v[1]] +
                (1 - b[0] - b[1]) * mesh->n[v[2]]);
        else
            it.n = Normalize(Normal3f(Cross(p1 - p0, p2 - p0)));
        if (reverseOrientation) it.n *= -1;
        Point3f pAbsSum = Abs(b[0] * p0) + Abs(b[1] * p1) +
            Abs((1 - b[0] - b[1]) * p2);
        it.pError = gamma(6) * Vector3f(pAbsSum);
        return it;
    }

    float Triangle::Area() const
    {
        Point3f p0 = mesh->p[v[0]], p1 = mesh->p[v[1]], p2 = mesh->p[v[2]];
        return 0.5 * Cross(p1 - p0, p2 - p0).Length();
    }

    bool Triangle::IntersectP(const Ray &ray, bool testAlphaTexture) const
    {
        return Shape::IntersectP(ray, testAlphaTexture);
    }

    std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(
            const Transform* o2w, const Transform* w2o, bool reverseOrientation,
            int nTriangles, const int* vertexIndices, int nVertices, const Point3f* p,
            const Vector3f* s, const Normal3f* n, const Point2f* uv,
            const std::shared_ptr<Texture<float>>& alphaTexture,
            const std::shared_ptr<Texture<float>>& shadowAlphaTexture,
            const int* faceIndices)
    {
        std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(
                *o2w, nTriangles, vertexIndices, nVertices, p, s, n, uv,
                alphaTexture, shadowAlphaTexture, faceIndices);
        std::vector<std::shared_ptr<Shape>> tris;
        tris.reserve(nTriangles);
        for (int i = 0; i < nTriangles; ++i)
            tris.emplace_back(std::make_shared<Triangle>(o2w, w2o,
                                                         reverseOrientation, mesh, i));
        return tris;
    }

    std::vector<std::shared_ptr<Shape>> CreateTriangleMeshShape(
            const Transform *o2w, const Transform *w2o, bool reverseOrientation,
            const ParamSet &params,
            std::map<std::string, std::shared_ptr<Texture<float>>> *floatTextures)
    {
        int nvi, npi, nuvi, nsi, nni;
        const int* vi = params.FindInt("indices", &nvi);
        const Point3f* P = params.FindPoint3f("P", &npi);
        const Point2f* uvs = params.FindPoint2f("uv", &nuvi);
        if (!uvs) uvs = params.FindPoint2f("st", &nuvi);
        std::unique_ptr<Point2f[]> tempUVs;
        if (!uvs)
        {
            const float* fuv = params.FindFloat("uv", &nuvi);
            if (!fuv) fuv = params.FindFloat("st", &nuvi);
            if (fuv) {
                nuvi /= 2;
                tempUVs = std::make_unique<Point2f[]>(nuvi);
                for (int i = 0; i < nuvi; ++i)
                    tempUVs[i] = Point2f(fuv[2 * i], fuv[2 * i + 1]);
                uvs = tempUVs.get();
            }
        }

        if (uvs)
        {
            if (nuvi < npi) {
                Error(
                        "Not enough of \"uv\"s for triangle mesh.  Expected %d, "
                        "found %d.  Discarding.",
                        npi, nuvi);
                uvs = nullptr;
            } else if (nuvi > npi)
                Warning(
                        "More \"uv\"s provided than will be used for triangle "
                        "mesh.  (%d expcted, %d found)",
                        npi, nuvi);
        }

        if (!vi) {
            Error(
                    "Vertex indices \"indices\" not provided with triangle mesh shape");
            return {};
        }
        if (!P) {
            Error("Vertex positions \"P\" not provided with triangle mesh shape");
            return {};
        }

        const Vector3f *S = params.FindVector3f("S", &nsi);
        if (S && nsi != npi) {
            Error("Number of \"S\"s for triangle mesh must match \"P\"s");
            S = nullptr;
        }
        const Normal3f *N = params.FindNormal3f("N", &nni);
        if (N && nni != npi) {
            Error("Number of \"N\"s for triangle mesh must match \"P\"s");
            N = nullptr;
        }
        for (int i = 0; i < nvi; ++i)
            if (vi[i] >= npi)
            {
                Error(
                        "trianglemesh has out of-bounds vertex index %d (%d \"P\" "
                        "values were given",
                        vi[i], npi);
                return {};
            }

        int nfi;
        const int *faceIndices = params.FindInt("faceIndices", &nfi);
        if (faceIndices && nfi != nvi / 3)
        {
            Error("Number of face indices, %d, doesn't match number of faces, %d",
                  nfi, nvi / 3);
            faceIndices = nullptr;
        }

        std::shared_ptr<Texture<float>> alphaTex;
        std::string alphaTexName = params.FindTexture("alpha");
        if (!alphaTexName.empty())
        {
            if (floatTextures->find(alphaTexName) != floatTextures->end())
                alphaTex = (*floatTextures)[alphaTexName];
            else
                Error("Couldn't find float texture \"%s\" for \"alpha\" parameter",
                      alphaTexName.c_str());
        }
        else if (params.FindOneFloat("alpha", 1.f) == 0.f)
            alphaTex.reset(new ConstantTexture<float>(0.f));

        std::shared_ptr<Texture<float>> shadowAlphaTex;
        std::string shadowAlphaTexName = params.FindTexture("shadowalpha");
        if (!alphaTexName.empty())
        {
            if (floatTextures->find(shadowAlphaTexName) != floatTextures->end())
                shadowAlphaTex = (*floatTextures)[shadowAlphaTexName];
            else
                Error(
                        "Couldn't find float texture \"%s\" for \"shadowalpha\" "
                        "parameter",
                        shadowAlphaTexName.c_str());
        } else if (params.FindOneFloat("shadowalpha", 1.f) == 0.f)
            shadowAlphaTex.reset(new ConstantTexture<float>(0.f));

        return CreateTriangleMesh(o2w, w2o, reverseOrientation, nvi / 3, vi, npi, P,
                                  S, N, uvs, alphaTex, shadowAlphaTex, faceIndices);
    }
}