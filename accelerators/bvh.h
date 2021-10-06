#ifndef PBRT_ACCELERATORS_BVH_H
#define PBRT_ACCELERATORS_BVH_H

#include "pbrt.h"
#include "primitive.h"

namespace pbrt
{
	struct BVHPrimitiveInfo;
	struct BVHBuildNode;

	class BVHAccel : public Aggregate
	{
	public:
		enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };

		BVHAccel(const std::vector<std::shared_ptr<Primitive>>& p,
		         int maxPrimsInNode, SplitMethod splitMethod);
	private:
		BVHBuildNode* recursiveBuild(MemoryArena& arena,
		                             std::vector<BVHPrimitiveInfo>& primitiveInfo,
		                             int start, int end, int* totalNodes,
		                             std::vector<std::shared_ptr<Primitive>>& orderedPrims);
		BVHBuildNode* HLBVHBuild(MemoryArena& arena,
		                         const std::vector<BVHPrimitiveInfo>& primitiveInfo,
		                         int* totalNodes,
		                         std::vector<std::shared_ptr<Primitive>>& orderedPrims) const;
		const int maxPrimsInNode;
		const SplitMethod splitMethod;
		std::vector<std::shared_ptr<Primitive>> primitives;
	};
}

#endif
