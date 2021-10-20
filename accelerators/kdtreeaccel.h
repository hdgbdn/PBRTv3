#ifndef PBRT_ACCELERATORS_KDTREEACCEL_H
#define PBRT_ACCELERATORS_KDTREEACCEL_H

#include "pbrt.h"
#include "primitive.h"

namespace pbrt
{
	struct KdAccelNode;
	struct BoundEdge;
	class KdTreeAccel : public Aggregate
	{
	public:
		KdTreeAccel(const std::vector<std::shared_ptr<Primitive>>& p,
		            int isectCost, int traversalCost, float emptyBonus,
		            int maxPrims, int maxDepth);
		void buildTree(int nodeNum, const Bounds3f& nodeBounds,
			const std::vector<Bounds3f>& allPrimBounds, int* primNums,
			int nPrimitives, int depth,
			const std::unique_ptr<BoundEdge[]> edges[3],
			int* prims0, int* prims1, int badRefines = 0);
		bool Intersect(const Ray& r, SurfaceInteraction* isect) const override;
		bool IntersectP(const Ray&) override;
	private:
		const int isectCost, traversalCost, maxPrimis;
		const float emptyBonus;
		std::vector<std::shared_ptr<Primitive>> primitives;
		std::vector<int> primitiveIndices;
		KdAccelNode* nodes;
		int nAllocedNodes, nextFreeNode;
		Bounds3f bounds;
	};
}

#endif