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
        faceIndex = mesh->faceIndices.size() ? mesh->faceIndices[triNumber] : 0;
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

        // Perform ray--triangle intersection test

        // Transform triangle vertices to ray coordinate space

        // Translate vertices based on ray origin
        Point3f p0t = p0 - Vector3f(ray.o);
        Point3f p1t = p1 - Vector3f(ray.o);
        Point3f p2t = p2 - Vector3f(ray.o);

        // Permute components of triangle vertices and ray direction
        int kz = MaxDimension(Abs(ray.d));
        int kx = kz + 1;
        if (kx == 3) kx = 0;
        int ky = kx + 1;
        if (ky == 3) ky = 0;
        Vector3f d = Permute(ray.d, kx, ky, kz);
        p0t = Permute(p0t, kx, ky, kz);
        p1t = Permute(p1t, kx, ky, kz);
        p2t = Permute(p2t, kx, ky, kz);

        // Apply shear transformation to translated vertex positions
        float Sx = -d.x / d.z;
        float Sy = -d.y / d.z;
        float Sz = 1.f / d.z;
        p0t.x += Sx * p0t.z;
        p0t.y += Sy * p0t.z;
        p1t.x += Sx * p1t.z;
        p1t.y += Sy * p1t.z;
        p2t.x += Sx * p2t.z;
        p2t.y += Sy * p2t.z;

        // Compute edge function coefficients _e0_, _e1_, and _e2_
        float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
        float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
        float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

        // Fall back to double precision test at triangle edges
        if (sizeof(float) == sizeof(float) &&
            (e0 == 0.0f || e1 == 0.0f || e2 == 0.0f)) {
            double p2txp1ty = (double)p2t.x * (double)p1t.y;
            double p2typ1tx = (double)p2t.y * (double)p1t.x;
            e0 = (float)(p2typ1tx - p2txp1ty);
            double p0txp2ty = (double)p0t.x * (double)p2t.y;
            double p0typ2tx = (double)p0t.y * (double)p2t.x;
            e1 = (float)(p0typ2tx - p0txp2ty);
            double p1txp0ty = (double)p1t.x * (double)p0t.y;
            double p1typ0tx = (double)p1t.y * (double)p0t.x;
            e2 = (float)(p1typ0tx - p1txp0ty);
        }

        // Perform triangle edge and determinant tests
        if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
            return false;
        float det = e0 + e1 + e2;
        if (det == 0) return false;

        // Compute scaled hit distance to triangle and test against ray $t$ range
        p0t.z *= Sz;
        p1t.z *= Sz;
        p2t.z *= Sz;
        float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
        if (det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det))
            return false;
        else if (det > 0 && (tScaled <= 0 || tScaled > ray.tMax * det))
            return false;

        // Compute barycentric coordinates and $t$ value for triangle intersection
        float invDet = 1 / det;
        float b0 = e0 * invDet;
        float b1 = e1 * invDet;
        float b2 = e2 * invDet;
        float t = tScaled * invDet;

        // Ensure that computed triangle $t$ is conservatively greater than zero

        // Compute $\delta_z$ term for triangle $t$ error bounds
        float maxZt = MaxComponent(Abs(Vector3f(p0t.z, p1t.z, p2t.z)));
        float deltaZ = gamma(3) * maxZt;

        // Compute $\delta_x$ and $\delta_y$ terms for triangle $t$ error bounds
        float maxXt = MaxComponent(Abs(Vector3f(p0t.x, p1t.x, p2t.x)));
        float maxYt = MaxComponent(Abs(Vector3f(p0t.y, p1t.y, p2t.y)));
        float deltaX = gamma(5) * (maxXt + maxZt);
        float deltaY = gamma(5) * (maxYt + maxZt);

        // Compute $\delta_e$ term for triangle $t$ error bounds
        float deltaE =
            2 * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

        // Compute $\delta_t$ term for triangle $t$ error bounds and check _t_
        float maxE = MaxComponent(Abs(Vector3f(e0, e1, e2)));
        float deltaT = 3 *
            (gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE) *
            std::abs(invDet);
        if (t <= deltaT) return false;

        // Compute triangle partial derivatives
        Vector3f dpdu, dpdv;
        Point2f uv[3];
        GetUVs(uv);

        // Compute deltas for triangle partial derivatives
        Vector2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
        Vector3f dp02 = p0 - p2, dp12 = p1 - p2;
        float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
        bool degenerateUV = std::abs(determinant) < 1e-8;
        if (!degenerateUV) {
            float invdet = 1 / determinant;
            dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
            dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
        }
        if (degenerateUV || Cross(dpdu, dpdv).LengthSquared() == 0) {
            // Handle zero determinant for triangle partial derivative matrix
            Vector3f ng = Cross(p2 - p0, p1 - p0);
            if (ng.LengthSquared() == 0)
                // The triangle is actually degenerate; the intersection is
                // bogus.
                return false;

            CoordinateSystem(Normalize(ng), &dpdu, &dpdv);
        }

        // Compute error bounds for triangle intersection
        float xAbsSum =
            (std::abs(b0 * p0.x) + std::abs(b1 * p1.x) + std::abs(b2 * p2.x));
        float yAbsSum =
            (std::abs(b0 * p0.y) + std::abs(b1 * p1.y) + std::abs(b2 * p2.y));
        float zAbsSum =
            (std::abs(b0 * p0.z) + std::abs(b1 * p1.z) + std::abs(b2 * p2.z));
        Vector3f pError = gamma(7) * Vector3f(xAbsSum, yAbsSum, zAbsSum);

        // Interpolate $(u,v)$ parametric coordinates and hit point
        Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
        Point2f uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];

        // Test intersection against alpha texture, if present
        if (testAlphaTexture && mesh->alphaMask) {
            SurfaceInteraction isectLocal(pHit, Vector3f(0, 0, 0), uvHit, -ray.d,
                dpdu, dpdv, Normal3f(0, 0, 0),
                Normal3f(0, 0, 0), ray.time, this);
            if (mesh->alphaMask->Evaluate(isectLocal) == 0) return false;
        }

        // Fill in _SurfaceInteraction_ from triangle hit
        *isect = SurfaceInteraction(pHit, pError, uvHit, -ray.d, dpdu, dpdv,
            Normal3f(0, 0, 0), Normal3f(0, 0, 0), ray.time,
            this);

        // Override surface normal in _isect_ for triangle
        isect->n = isect->shading.n = Normal3f(Normalize(Cross(dp02, dp12)));
        if (reverseOrientation ^ transformSwapsHandedness)
            isect->n = isect->shading.n = -isect->n;

        if (mesh->n || mesh->s) {
            // Initialize _Triangle_ shading geometry

            // Compute shading normal _ns_ for triangle
            Normal3f ns;
            if (mesh->n) {
                ns = (b0 * mesh->n[v[0]] + b1 * mesh->n[v[1]] + b2 * mesh->n[v[2]]);
                if (ns.LengthSquared() > 0)
                    ns = Normalize(ns);
                else
                    ns = isect->n;
            }
            else
                ns = isect->n;

            // Compute shading tangent _ss_ for triangle
            Vector3f ss;
            if (mesh->s) {
                ss = (b0 * mesh->s[v[0]] + b1 * mesh->s[v[1]] + b2 * mesh->s[v[2]]);
                if (ss.LengthSquared() > 0)
                    ss = Normalize(ss);
                else
                    ss = Normalize(isect->dpdu);
            }
            else
                ss = Normalize(isect->dpdu);

            // Compute shading bitangent _ts_ for triangle and adjust _ss_
            Vector3f ts = Cross(ss, ns);
            if (ts.LengthSquared() > 0.f) {
                ts = Normalize(ts);
                ss = Cross(ts, ns);
            }
            else
                CoordinateSystem((Vector3f)ns, &ss, &ts);

            // Compute $\dndu$ and $\dndv$ for triangle shading geometry
            Normal3f dndu, dndv;
            if (mesh->n) {
                // Compute deltas for triangle partial derivatives of normal
                Vector2f duv02 = uv[0] - uv[2];
                Vector2f duv12 = uv[1] - uv[2];
                Normal3f dn1 = mesh->n[v[0]] - mesh->n[v[2]];
                Normal3f dn2 = mesh->n[v[1]] - mesh->n[v[2]];
                float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
                bool degenerateUV = std::abs(determinant) < 1e-8;
                if (degenerateUV) {
                    // We can still compute dndu and dndv, with respect to the
                    // same arbitrary coordinate system we use to compute dpdu
                    // and dpdv when this happens. It's important to do this
                    // (rather than giving up) so that ray differentials for
                    // rays reflected from triangles with degenerate
                    // parameterizations are still reasonable.
                    Vector3f dn = Cross(Vector3f(mesh->n[v[2]] - mesh->n[v[0]]),
                        Vector3f(mesh->n[v[1]] - mesh->n[v[0]]));
                    if (dn.LengthSquared() == 0)
                        dndu = dndv = Normal3f(0, 0, 0);
                    else {
                        Vector3f dnu, dnv;
                        CoordinateSystem(dn, &dnu, &dnv);
                        dndu = Normal3f(dnu);
                        dndv = Normal3f(dnv);
                    }
                }
                else {
                    float invDet = 1 / determinant;
                    dndu = (duv12[1] * dn1 - duv02[1] * dn2) * invDet;
                    dndv = (-duv12[0] * dn1 + duv02[0] * dn2) * invDet;
                }
            }
            else
                dndu = dndv = Normal3f(0, 0, 0);
            if (reverseOrientation) ts = -ts;
            isect->SetShadingGeometry(ss, ts, dndu, dndv, true);
        }

        *tHit = t;
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