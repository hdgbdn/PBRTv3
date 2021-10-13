#ifndef PBRT_ACCELERATORS_BVH_H
#define PBRT_ACCELERATORS_BVH_H

#include "pbrt.h"
#include "primitive.h"

namespace pbrt
{
	struct BVHPrimitiveInfo;
	struct BVHBuildNode;
	struct MortonPrimitive;
	struct LinearBVHNode;

	class BVHAccel : public Aggregate
	{
	public:
		enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };

		BVHAccel(const std::vector<std::shared_ptr<Primitive>>& p,
		         int maxPrimsInNode, SplitMethod splitMethod);
		Bounds3f WorldBound() const override;
		~BVHAccel();
		bool Intersect(const Ray& r, SurfaceInteraction* isect) const override;
		bool IntersectP(const Ray&) override;
	private:
		BVHBuildNode* recursiveBuild(MemoryArena& arena,
		                             std::vector<BVHPrimitiveInfo>& primitiveInfo,
		                             int start, int end, int* totalNodes,
		                             std::vector<std::shared_ptr<Primitive>>& orderedPrims);
		BVHBuildNode* emitLBVH(BVHBuildNode*& buildNodes,
			const std::vector<BVHPrimitiveInfo>& primitiveInfo,
			MortonPrimitive* mortonPrims, int nPrimitives, int* totalNodes,
			std::vector<std::shared_ptr<Primitive>>& orderedPrims,
			std::atomic<int>* orderedPrimsOffset, int bitIndex) const;
		BVHBuildNode* buildUpperSAH(MemoryArena& arena,
			std::vector<BVHBuildNode*>& treeletRoots, int start, int end,
			int* totalNodes) const;
		int flattenBVHTree(BVHBuildNode* node, int* offset);
		BVHBuildNode* HLBVHBuild(MemoryArena& arena,
		                         const std::vector<BVHPrimitiveInfo>& primitiveInfo,
		                         int* totalNodes,
		                         std::vector<std::shared_ptr<Primitive>>& orderedPrims) const;
		const int maxPrimsInNode;
		const SplitMethod splitMethod;
		std::vector<std::shared_ptr<Primitive>> primitives;
		LinearBVHNode* linearNodes = nullptr;
	};
}

#endif
