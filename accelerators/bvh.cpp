#include "bvh.h"
#include "memory.h"
#include "parallel.h"

namespace pbrt
{
	inline uint32_t LeftShift3(uint32_t x) {
		//CHECK_LE(x, (1 << 10));
		if (x == (1 << 10)) --x;

		x = (x | (x << 16)) & 0b00000011000000000000000011111111;
		// x = ---- --98 ---- ---- ---- ---- 7654 3210
		x = (x | (x << 8)) & 0b00000011000000001111000000001111;
		// x = ---- --98 ---- ---- 7654 ---- ---- 3210
		x = (x | (x << 4)) & 0b00000011000011000011000011000011;
		// x = ---- --98 ---- 76-- --54 ---- 32-- --10
		x = (x | (x << 2)) & 0b00001001001001001001001001001001;
		// x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0

		return x;
	}

	inline uint32_t EncodeMorton3(const Vector3f& v) {
		//CHECK_GE(v.x, 0);
		//CHECK_GE(v.y, 0);
		//CHECK_GE(v.z, 0);
		return (LeftShift3(v.z) << 2) | (LeftShift3(v.y) << 1) | LeftShift3(v.x);
	}

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

	struct LinearBVHNode {
		Bounds3f bounds;
		union {
			int primitivesOffset;   // leaf
			int secondChildOffset;  // interior
		};
		uint16_t nPrimitives;  // 0 -> interior node
		uint8_t axis;          // interior node: xyz
		uint8_t pad[1];        // ensure 32 byte total size
	};

	struct MortonPrimitive {
		int primitiveIndex;
		uint32_t mortonCode;
	};

	struct LBVHTreelet {
		int startIndex, nPrimitives;
		BVHBuildNode* buildNodes;
	};

	static void RadixSort(std::vector<MortonPrimitive>* v)
	{
		std::vector<MortonPrimitive> tempVector(v->size());
		constexpr int bitsPerPass = 6;
		constexpr int nBits = 30;
		constexpr int nPasses = nBits / bitsPerPass;
		for (int pass = 0; pass < nPasses; ++pass)
		{
			int lowBit = pass * bitsPerPass;
			std::vector<MortonPrimitive>& in  = (pass & 1) ? tempVector : *v;
			std::vector<MortonPrimitive>& out = (pass & 1) ? *v : tempVector;
			constexpr int nBuckets = 1 << bitsPerPass;
			int bucketCount[nBuckets] = { 0 };
			constexpr int bitMask = (1 << bitsPerPass) - 1;
			for(const auto& mp: in)
			{
				int bucket = (mp.mortonCode >> lowBit) & bitMask;
				++bucketCount[bucket];
			}
			int outIndex[nBuckets];
			outIndex[0] = 0;
			for (int i = 1; i < nBuckets; ++i)
				outIndex[i] = outIndex[i - 1] + bucketCount[i - 1];
			for (const MortonPrimitive& mp : in) {
				int bucket = (mp.mortonCode >> lowBit) & bitMask;
				out[outIndex[bucket]++] = mp;
			}
		}
		if (nPasses & 1)
			std::swap(*v, tempVector);
	}

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
		primitiveInfo.resize(0);
		linearNodes = AllocAligned<LinearBVHNode>(totalNodes);
		int offset = 0;
		flattenBVHTree(root, &offset);
	}

	struct BucketInfo {
		int count = 0;
		Bounds3f bounds;
	};

	BVHBuildNode* BVHAccel::recursiveBuild(
		MemoryArena& arena, std::vector<BVHPrimitiveInfo>& primitiveInfo, int start,
		int end, int* totalNodes,
		std::vector<std::shared_ptr<Primitive>>& orderedPrims) {
		//CHECK_NE(start, end);
		BVHBuildNode* node = arena.Alloc<BVHBuildNode>();
		(*totalNodes)++;
		// Compute bounds of all primitives in BVH node
		Bounds3f bounds;
		for (int i = start; i < end; ++i)
			bounds = Union(bounds, primitiveInfo[i].bounds);
		int nPrimitives = end - start;
		if (nPrimitives == 1) {
			// Create leaf _BVHBuildNode_
			int firstPrimOffset = orderedPrims.size();
			int primNum = primitiveInfo[start].primitiveNumber;
			orderedPrims.push_back(primitives[primNum]);
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else {
			// Compute bound of primitive centroids, choose split dimension _dim_
			Bounds3f centroidBounds;
			for (int i = start; i < end; ++i)
				centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
			int dim = centroidBounds.MaximumExtent();

			// Partition primitives into two sets and build children
			int mid = (start + end) / 2;
			if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim]) {
				// Create leaf _BVHBuildNode_
				int firstPrimOffset = orderedPrims.size();
				for (int i = start; i < end; ++i) {
					int primNum = primitiveInfo[i].primitiveNumber;
					orderedPrims.push_back(primitives[primNum]);
				}
				node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
				return node;
			}
			else 
			{
				// Partition primitives based on _splitMethod_
				switch (splitMethod) {
				case SplitMethod::Middle: {
					// Partition primitives through node's midpoint
					float pmid =
						(centroidBounds.pMin[dim] + centroidBounds.pMax[dim]) / 2;
					BVHPrimitiveInfo* midPtr = std::partition(
						&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
						[dim, pmid](const BVHPrimitiveInfo& pi) {
							return pi.centroid[dim] < pmid;
						});
					mid = midPtr - &primitiveInfo[0];
					// For lots of prims with large overlapping bounding boxes, this
					// may fail to partition; in that case don't break and fall
					// through
					// to EqualCounts.
					if (mid != start && mid != end) break;
				}
				case SplitMethod::EqualCounts: {
					// Partition primitives into equally-sized subsets
					mid = (start + end) / 2;
					std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
						&primitiveInfo[end - 1] + 1,
						[dim](const BVHPrimitiveInfo& a,
							const BVHPrimitiveInfo& b) {
								return a.centroid[dim] < b.centroid[dim];
						});
					break;
				}
				case SplitMethod::SAH:
				default: {
					// Partition primitives using approximate SAH
					if (nPrimitives <= 2) {
						// Partition primitives into equally-sized subsets
						mid = (start + end) / 2;
						std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
							&primitiveInfo[end - 1] + 1,
							[dim](const BVHPrimitiveInfo& a,
								const BVHPrimitiveInfo& b) {
									return a.centroid[dim] <
										b.centroid[dim];
							});
					}
					else {
						// Allocate _BucketInfo_ for SAH partition buckets
						static constexpr int nBuckets = 12;
						BucketInfo buckets[nBuckets];

						// Initialize _BucketInfo_ for SAH partition buckets
						for (int i = start; i < end; ++i) {
							int b = nBuckets *
								centroidBounds.Offset(
									primitiveInfo[i].centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							//CHECK_GE(b, 0);
							//CHECK_LT(b, nBuckets);
							buckets[b].count++;
							buckets[b].bounds =
								Union(buckets[b].bounds, primitiveInfo[i].bounds);
						}

						// Compute costs for splitting after each bucket
						// and Find bucket to split at that minimizes SAH metric
						float cost[nBuckets - 1];
						float minCost = std::numeric_limits<float>::max();
						int minCostSplitBucket = 0;
						for (int i = 0; i < nBuckets - 1; ++i) {
							Bounds3f b0, b1;
							int count0 = 0, count1 = 0;
							for (int j = 0; j <= i; ++j) {
								b0 = Union(b0, buckets[j].bounds);
								count0 += buckets[j].count;
							}
							for (int j = i + 1; j < nBuckets; ++j) {
								b1 = Union(b1, buckets[j].bounds);
								count1 += buckets[j].count;
							}
							cost[i] = 1 +
								(count0 * b0.SurfaceArea() +
									count1 * b1.SurfaceArea()) /
								bounds.SurfaceArea();
							if (cost[i] < minCost) {
								minCost = cost[i];
								minCostSplitBucket = i;
							}
						}

						// Either create leaf or split primitives at selected SAH
						// bucket
						float leafCost = nPrimitives;
						if (nPrimitives > maxPrimsInNode || minCost < leafCost) {
							BVHPrimitiveInfo* pmid = std::partition(
								&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
								[=](const BVHPrimitiveInfo& pi) {
									int b = nBuckets *
										centroidBounds.Offset(pi.centroid)[dim];
									if (b == nBuckets) b = nBuckets - 1;
									//CHECK_GE(b, 0);
									//CHECK_LT(b, nBuckets);
									return b <= minCostSplitBucket;
								});
							mid = pmid - &primitiveInfo[0];
						}
						else {
							// Create leaf _BVHBuildNode_
							int firstPrimOffset = orderedPrims.size();
							for (int i = start; i < end; ++i) {
								int primNum = primitiveInfo[i].primitiveNumber;
								orderedPrims.push_back(primitives[primNum]);
							}
							node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
							return node;
						}
					}
					break;
				}
				}
				auto left = recursiveBuild(arena, primitiveInfo, start, mid,
					totalNodes, orderedPrims);
				auto right = recursiveBuild(arena, primitiveInfo, mid, end,
					totalNodes, orderedPrims);
				node->InitInterior(dim, left, right);
			}
		}
		return node;
	}



	BVHBuildNode* pbrt::BVHAccel::emitLBVH(BVHBuildNode*& buildNodes,
	                                       const std::vector<BVHPrimitiveInfo>& primitiveInfo,
	                                       MortonPrimitive* mortonPrims, int nPrimitives, int* totalNodes,
	                                       std::vector<std::shared_ptr<Primitive>>& orderedPrims,
	                                       std::atomic<int>* orderedPrimsOffset, int bitIndex) const
	{
		if(bitIndex == -1 || nPrimitives < maxPrimsInNode)
		{
			(*totalNodes)++;
			BVHBuildNode* node = buildNodes++;
			Bounds3f bounds;
			int firstPrimOffset = orderedPrimsOffset->fetch_add(nPrimitives);
			for (int i = 0; i < nPrimitives; ++i)
			{
				int primitiveIndex = mortonPrims[i].primitiveIndex;
				orderedPrims[firstPrimOffset + i] = primitives[primitiveIndex];
				bounds = Union(bounds, primitiveInfo[primitiveIndex].bounds);
			}
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else
		{
			int mask = 1 << bitIndex;
			if ((mortonPrims[0].mortonCode & mask) ==
				(mortonPrims[nPrimitives - 1].mortonCode & mask))
				return emitLBVH(buildNodes, primitiveInfo, mortonPrims, nPrimitives,
				                totalNodes, orderedPrims, orderedPrimsOffset, bitIndex - 1);
			int searchStart = 0, searchEnd = nPrimitives - 1;
			while(searchStart + 1 != searchEnd)
			{
				int mid = (searchStart + searchEnd) / 2;
				if ((mortonPrims[searchStart].mortonCode & mask) ==
					(mortonPrims[mid].mortonCode & mask))
					searchStart = mid;
				else
					searchEnd = mid;
			}
			int splitOffset = searchEnd;
			(*totalNodes)++;
			BVHBuildNode* node = buildNodes++;
			BVHBuildNode* lbvh[2] = {
		   emitLBVH(buildNodes, primitiveInfo, mortonPrims, splitOffset,
					totalNodes, orderedPrims, orderedPrimsOffset,
					bitIndex - 1),
		   emitLBVH(buildNodes, primitiveInfo, &mortonPrims[splitOffset],
					nPrimitives - splitOffset, totalNodes, orderedPrims,
					orderedPrimsOffset, bitIndex - 1) };
			int axis = bitIndex % 3;
			node->InitInterior(axis, lbvh[0], lbvh[1]);
			return node;
		}
	}

	BVHBuildNode* BVHAccel::buildUpperSAH(MemoryArena& arena, std::vector<BVHBuildNode*>& treeletRoots,
		int start, int end, int* totalNodes) const
	{
		int nNodes = end - start;
		if (nNodes == 1) return treeletRoots[start];
		(*totalNodes)++;
		BVHBuildNode* node = arena.Alloc<BVHBuildNode>();
		Bounds3f bounds;
		for (int i = start; i < end; ++i)
			bounds = Union(bounds, treeletRoots[i]->bounds);

		Bounds3f centroidBounds;
		for(int i = start; i < end; ++i)
		{
			Point3f centroid =
				(treeletRoots[i]->bounds.pMin + treeletRoots[i]->bounds.pMax) * 0.5;
			centroidBounds.Union(centroid);
		}
		int dim = centroidBounds.MaximumExtent();
		constexpr int nBuckets = 12;
		struct BucketInfo
		{
			int count = 0;
			Bounds3f bounds;
		};
		BucketInfo buckets[nBuckets];
		for (int i = start; i < end; ++i)
		{
			float centroid =
				(treeletRoots[i]->bounds.pMin[dim] + treeletRoots[i]->bounds.pMax[dim]) * 0.5;
			int b = nBuckets *
				centroidBounds.Offset(centroid)[dim];
			if (b == nBuckets) b = nBuckets - 1;
			buckets[b].count++;
			buckets[b].bounds.Union(treeletRoots[i]->bounds);
		}
		float cost[nBuckets - 1];
		for (int i = 0; i < nBuckets - 1; ++i)
		{
			Bounds3f b0, b1;
			int count0 = 0, count1 = 0;
			for (int j = 0; j <= i; ++j)
			{
				b0 = Union(b0, buckets[j].bounds);
				count0 += buckets[j].count;
			}
			for (int j = i + 1; j < nBuckets; ++j)
			{
				b1 = Union(b0, buckets[j].bounds);
				count1 += buckets[j].count;
			}
			cost[i] = .125f + (count0 * b0.SurfaceArea() +
				count1 * b1.SurfaceArea()) / bounds.SurfaceArea();
		}
		float minCost = cost[0];
		int minCostSplitBucket = 0;
		for (int i = 1; i < nBuckets - 1; ++i)
		{
			if (cost[i] < minCost)
			{
				float minCost = cost[i];
				int minCostSplitBucket = i;
			}
		}
		BVHBuildNode** pmid = std::partition(
			&treeletRoots[start], &treeletRoots[end - 1] + 1,
			[=](const BVHBuildNode* node)
			{
				float centroid =
					(node->bounds.pMin[dim] + node->bounds.pMax[dim]) * 0.5f;
				int b = nBuckets *
					((centroid - centroidBounds.pMin[dim]) /
						(centroidBounds.pMax[dim] - centroidBounds.pMin[dim]));
				if (b == nBuckets) b = nBuckets - 1;
				return b <= minCostSplitBucket;
			});
		int mid = pmid - &treeletRoots[0];
		node->InitInterior(dim,
			this->buildUpperSAH(arena, treeletRoots, start, mid, totalNodes),
			this->buildUpperSAH(arena, treeletRoots, mid, end, totalNodes));
		return node;
	}

	BVHBuildNode* BVHAccel::HLBVHBuild(MemoryArena& arena, const std::vector<BVHPrimitiveInfo>& primitiveInfo,
	                                   int* totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrims) const
	{
		Bounds3f bounds;
		for (auto& pi : primitiveInfo)
			bounds = Union(bounds, pi.centroid);
		std::vector<MortonPrimitive> mortonPrims(primitiveInfo.size());
		ParallelFor(
			[&](int i)
			{
				constexpr int mortonBits = 10;
				constexpr int mortonScale = 1 << mortonBits;
				mortonPrims[i].primitiveIndex = primitiveInfo[i].primitiveNumber;
				Vector3f centroidOffset = bounds.Offset(primitiveInfo[i].centroid);
				mortonPrims[i].mortonCode = EncodeMorton3(centroidOffset * mortonScale);
			}, primitiveInfo.size(), 512);
		RadixSort(&mortonPrims);
		std::vector<LBVHTreelet> treeletsToBuild;
		for(int start = 0, end = 1; end <= (int)mortonPrims.size(); ++end)
		{
			uint32_t mask = 0b00111111111111000000000000000000;
			if (end == (int)mortonPrims.size() ||
				((mortonPrims[start].mortonCode & mask) !=
					(mortonPrims[end].mortonCode & mask)))
			{
				int nPrimitives = end - start;
				int maxBVHNodes = 2 * nPrimitives - 1;
				BVHBuildNode* nodes = arena.Alloc<BVHBuildNode>(maxBVHNodes, false);
				treeletsToBuild.push_back({ start, nPrimitives, nodes });
				start = end;
			}
			std::atomic<int> atomicTotal(0), orderedPrimsOffset(0);
			orderedPrims.resize(primitives.size());
			ParallelFor(
				[&](int i)
				{
					int nodesCreated = 0;
					const int firstBitIndex = 29 - 12;
					LBVHTreelet& tr = treeletsToBuild[i];
					tr.buildNodes = emitLBVH(tr.buildNodes, primitiveInfo,
						&mortonPrims[tr.startIndex], tr.nPrimitives,
						&nodesCreated, orderedPrims, &orderedPrimsOffset, firstBitIndex);
					atomicTotal += nodesCreated;
				}, treeletsToBuild.size());
			*totalNodes = atomicTotal;
		}
		std::vector<BVHBuildNode*> finishedTreelets;
		for (LBVHTreelet& treelet : treeletsToBuild)
			finishedTreelets.push_back(treelet.buildNodes);
		return buildUpperSAH(arena, finishedTreelets, 0,
			finishedTreelets.size(), totalNodes);
	}


	int BVHAccel::flattenBVHTree(BVHBuildNode* node, int* offset)
	{
		LinearBVHNode* linearNode = &linearNodes[*offset];
		linearNode->bounds = node->bounds;
		int myOffset = (*offset)++;
		if(node->nPrimitives > 0)
		{
			linearNode->nPrimitives = node->nPrimitives;
			linearNode->primitivesOffset = node->firstPrimOffset;
		}
		else
		{
			linearNode->axis = node->splitAxis;
			linearNode->nPrimitives = 0;
			flattenBVHTree(node->children[0], offset);
			int secondOffset = flattenBVHTree(node->children[1], offset);
			linearNode->secondChildOffset = secondOffset;
		}
		return myOffset;
	}

	Bounds3f BVHAccel::WorldBound() const
	{
		return linearNodes ? linearNodes[0].bounds : Bounds3f();
	}

	bool BVHAccel::Intersect(const Ray& r, SurfaceInteraction* isect) const
	{
		bool hit = false;
		Vector3f invDir(1 / r.d.x, 1 / r.d.y, 1 / r.d.z);
		int dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };
		int toVisitOffset = 0, currentNodeIndex = 0;
		int nodesToVisit[64];
		while(true)
		{
			const LinearBVHNode* curLinearNode = &linearNodes[currentNodeIndex];
			if(curLinearNode->bounds.IntersectP(r, invDir, dirIsNeg))
			{
				if (curLinearNode->nPrimitives > 0)
				{
					for (int i = 0; i < curLinearNode->nPrimitives; ++i)
						if (primitives[curLinearNode->primitivesOffset + i]->Intersect(r, isect))
							hit = true;
					if (toVisitOffset == 0) break;
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else
				{
					if (dirIsNeg[curLinearNode->axis]) {
						nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
						currentNodeIndex = curLinearNode->secondChildOffset;
					}
					else {
						nodesToVisit[toVisitOffset++] = curLinearNode->secondChildOffset;
						currentNodeIndex = currentNodeIndex + 1;
					}
				}
			}
			else
			{
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
		}
		return hit;
	}

    BVHAccel::~BVHAccel()
    {
        FreeAligned(linearNodes);
    }

    bool BVHAccel::IntersectP(const Ray &)
    {
        // TODO need implement
        return false;
    }


}
