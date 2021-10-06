#include "bvh.h"
#include "memory.h"
#include <algorithm>

namespace pbrt
{
	struct BVHPrimitiveInfo
	{
		BVHPrimitiveInfo() = default;

		BVHPrimitiveInfo(size_t primitiveNumber, const Bounds3f& bounds)
			: primitiveNumber(primitiveNumber),
			  bounds(bounds),
			  centroid(.5f * bounds.pMin + .5f * bounds.pMax)
		{
		}

		size_t primitiveNumber;
		Bounds3f bounds;
		Point3f centroid;
	};

	struct BVHBuildNode
	{
		void InitLeaf(int first, int n, const Bounds3f& b)
		{
			firstPrimOffset = first;
			nPrimitives = n;
			bounds = b;
			children[0] = children[1] = nullptr;
		}

		void InitInterior(int axis, BVHBuildNode* c0, BVHBuildNode* c1)
		{
			children[0] = c0;
			children[1] = c1;
			bounds = Union(c0->bounds, c1->bounds);
			splitAxis = axis;
			nPrimitives = 0;
		}

		Bounds3f bounds;
		BVHBuildNode* children[2];
		int splitAxis, firstPrimOffset, nPrimitives;
	};

	BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive>>& p, int maxPrimsInNode, SplitMethod splitMethod)
		: primitives(p), maxPrimsInNode(maxPrimsInNode), splitMethod(splitMethod)
	{
		std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
		for (size_t i = 0; i < primitives.size(); ++i)
		{
			primitiveInfo[i] = {i, primitives[i]->WorldBound()};
		}
		MemoryArena arena(1024 * 1024);
		int totalNodes = 0;
		std::vector<std::shared_ptr<Primitive>> orderedPrims;
		BVHBuildNode* root;
		if (splitMethod == SplitMethod::HLBVH)
			root = HLBVHBuild(arena, primitiveInfo, &totalNodes, orderedPrims);
		else
			root = recursiveBuild(arena, primitiveInfo, 0, primitives.size(),
			                      &totalNodes, orderedPrims);
		std::swap(primitives, orderedPrims);
	}

	BVHBuildNode* BVHAccel::recursiveBuild(MemoryArena& arena, std::vector<BVHPrimitiveInfo>& primitiveInfo,
	                                       int start, int end, int* totalNodes,
	                                       std::vector<std::shared_ptr<Primitive>>& orderedPrims)
	{
		BVHBuildNode* node = arena.Alloc<BVHBuildNode>();
		++(*totalNodes);
		Bounds3f bounds;
		for (int i = start; i < end; ++i)
			bounds = Union(bounds, primitiveInfo[i].bounds);
		int nPrimitives = end - start;
		if(nPrimitives == 1)
		{
			int firstPrimOffset = orderedPrims.size();
			for(int i = start; i < end; ++i)
			{
				int primNum = primitiveInfo[i].primitiveNumber;
				orderedPrims.push_back(primitives[primNum]);
			}
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else
		{
			Bounds3f centroidBounds;
			for (int i = start; i < end; ++i)
				centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
			int dim = centroidBounds.MaximumExtent();
			int mid = start + end / 2;
			if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim])
			{
			}
			else
			{
				// TODO only implement middle split method
				switch (splitMethod)
				{
				case SplitMethod::Middle:
					{
						float pMid = (centroidBounds.pMax[dim] + centroidBounds.pMin[dim]) / 2;
						auto midIter = std::partition(primitiveInfo.begin() + start,
						                              primitiveInfo.begin() + end,
						                              [dim, pMid](const std::vector<BVHPrimitiveInfo>::iterator& iter)
						                              {
							                              return iter->centroid[dim] < pMid;
						                              });
						mid = midIter - primitiveInfo.begin();
						if (mid != start && mid != end)
							break;
					}
				case SplitMethod::EqualCounts:
					{
						mid = start + end / 2;
						std::nth_element(primitiveInfo.begin() + start, primitiveInfo.begin() + mid,
						                 primitiveInfo.begin() + end,
						                 [dim](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b)
						                 {
							                 return a.centroid[dim] < b.centroid[dim];
						                 });
						break;
					}
				case SplitMethod::SAH:
				{
					if(nPrimitives <= 4)
					{
						mid = start + end / 2;
						std::nth_element(primitiveInfo.begin() + start, primitiveInfo.begin() + mid,
							primitiveInfo.begin() + end,
							[dim](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b)
							{
								return a.centroid[dim] < b.centroid[dim];
							});
					}
					else
					{
						constexpr int nBuckets = 12;
						struct BucketInfo
						{
							int count = 0;
							Bounds3f bounds;
						};
						BucketInfo buckets[nBuckets];
						for (int i = start; i < end; ++i)
						{
							int b = nBuckets *
								centroidBounds.Offset(primitiveInfo[i].centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							buckets[b].count++;
							buckets[b].bounds = Union(buckets[b].bounds, primitiveInfo[i].bounds);
						}
						float cost[nBuckets - 1];
						for(int i = 0; i < nBuckets - 1; ++i)
						{
							Bounds3f b0, b1;
							int count0 = 0, count1 = 0;
							for(int j = 0; j <= i; ++j)
							{
								b0 = Union(b0, buckets[j].bounds);
								count0 += buckets[j].count;
							}
							for(int j = i+1; j < nBuckets; ++j)
							{
								b1 = Union(b0, buckets[j].bounds);
								count1 += buckets[j].count;
							}
							cost[i] = .125f + (count0 * b0.SurfaceArea() +
								count1 * b1.SurfaceArea()) / bounds.SurfaceArea();
						}
						float minCost = cost[0];
						int minCostSplitBucket = 0;
						for(int i = 1; i < nBuckets - 1; ++i)
						{
							if(cost[i]< minCost)
							{
								float minCost = cost[i];
								int minCostSplitBucket = i;
							}
						}
						float leafCost = nPrimitives;
						if (nPrimitives > maxPrimsInNode && minCost < leafCost)
						{
							int firstPrimOffset = orderedPrims.size();
							for (int i = start; i < end; ++i)
							{
								int primNum = primitiveInfo[i].primitiveNumber;
								orderedPrims.push_back(primitives[primNum]);
							}
							node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
							return node;
							
						}
						else
						{
							auto midIter = std::partition(primitiveInfo.begin() + start,
								primitiveInfo.begin() + end,
								[=](const std::vector<BVHPrimitiveInfo>::iterator& iter)
								{
									int b = nBuckets * centroidBounds.Offset(iter->centroid)[
										dim];
									if (b == nBuckets) b = nBuckets - 1;
									return b <= minCostSplitBucket;
								});
							mid = midIter - primitiveInfo.begin();
						}
					}
				}
				}
				node->InitInterior(dim,
				                   recursiveBuild(arena, primitiveInfo, start, mid, totalNodes, orderedPrims),
				                   recursiveBuild(arena, primitiveInfo, mid, end, totalNodes, orderedPrims));
			}
		}
		return node;
	}
}
