#ifndef PBRT_SHAPE_TRIANGLE_H
#define PBRT_SHAPE_TRIANGLE_H

#include "core/shape.h"

namespace pbrt
{
	struct TriangleMesh
	{
		TriangleMesh(const Transform &ObjectToWorld, int nTriangles,
                     const int* vertexIndices, int nVertices, const Point3f* P,
                     const Vector3f* S, const Normal3f* N, const Point2f* UV,
                     std::shared_ptr<Texture<float>>  alphaMask,
                     std::shared_ptr<Texture<float>>  shadowAlphaMask,
                     const int *fIndices);
		const int nTriangles, nVertices;
		std::vector<int> vertexIndices;
		std::unique_ptr<Point3f[]> p;
		std::unique_ptr<Normal3f[]> n;
		std::unique_ptr<Vector3f[]> s;
		std::unique_ptr<Point2f[]> uv;
		std::shared_ptr<Texture<float>> alphaMask, shadowAlphaMask;
        std::vector<int> faceIndices;
	};

	class Triangle : public Shape
	{
	public:
		~Triangle() override;
		Triangle(const Transform *ObjectToWorld,
                 const Transform *WorldToObject, bool reverseOrientation,
                 const std::shared_ptr<TriangleMesh>& mesh, int triNumber);
		Bounds3f ObjectBound() const override;
		Bounds3f WorldBound() const override;
		bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		float Area() const override;
		Interaction Sample(const Point2f& u) const override;
	private:
		void GetUVs(Point2f uv[3]) const
		{
			if(mesh->uv)
			{
				uv[0] = mesh->uv[v[0]];
				uv[1] = mesh->uv[v[1]];
				uv[2] = mesh->uv[v[2]];
			}
			else
			{
				uv[0] = Point2f(0, 0);
				uv[1] = Point2f(1, 0);
				uv[2] = Point2f(1, 1);
			}
		}

		std::shared_ptr<TriangleMesh> mesh;
		const int* v;
	};

    std::vector<std::shared_ptr<Shape>> CreateTriangleMeshShape(
            const Transform *o2w, const Transform *w2o, bool reverseOrientation,
            const ParamSet &params,
            std::map<std::string, std::shared_ptr<Texture<float>>> *floatTextures);

	std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(
		const Transform* o2w, const Transform* w2o, bool reverseOrientation,
		int nTriangles, const int* vertexIndices, int nVertices, const Point3f* p,
		const Vector3f* s, const Normal3f* n, const Point2f* uv,
		const std::shared_ptr<Texture<float>>& alphaTexture,
		const std::shared_ptr<Texture<float>>& shadowAlphaTexture,
		const int* faceIndices = nullptr);
}

#endif