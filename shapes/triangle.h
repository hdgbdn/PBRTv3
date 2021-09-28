#ifndef PBRT_SHAPE_SPHERE_H
#define PBRT_SHAPE_SPHERE_H

#include "pbrt.h"
#include "transformation.h"

namespace pbrt
{
	struct TriangleMesh
	{
		TriangleMesh(const Transform& ObjectToWorld, int nTriangles,
			const int* vertexIndices, int nVertices, const Point3f* P,
			const Vector3f* S, const Normal3f* N, const Point2f* UV,
			const std::shared_ptr<Texture<float>>& alphaMask);
		const int nTriangles, nVertices;
		std::vector<int> vertexIndices;
		std::unique_ptr<Point3f[]> p;
		std::unique_ptr<Normal3f[]> n;
		std::unique_ptr<Vector3f[]> s;
		std::unique_ptr<Point2f[]> uv;
		std::shared_ptr<Texture<float>> alphaMask;
	};

	class Triangle : public Shape
	{
	public:
		Triangle(const std::shared_ptr<Transform>& ObjectToWorld,
			const std::shared_ptr<Transform>& WorldToObject, bool reverseOrientation,
			const std::shared_ptr<TriangleMesh>& mesh, int triNumber);
		Bounds3f ObjectBound() const override;
		Bounds3f WorldBound() const override;
		bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		float Area() override;
	private:
		std::shared_ptr<TriangleMesh> mesh;
		const int* v;
	};

	std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(
		const Transform* o2w, const Transform* w2o, bool reverseOrientation,
		int nTriangles, const int* vertexIndices, int nVertices, const Point3f* p,
		const Vector3f* s, const Normal3f* n, const Point2f* uv,
		const std::shared_ptr<Texture<float>>& alphaTexture,
		const std::shared_ptr<Texture<float>>& shadowAlphaTexture,
		const int* faceIndices = nullptr);
}

#endif