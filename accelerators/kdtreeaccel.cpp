#include "kdtreeaccel.h"
#include "memory.h"

namespace pbrt
{
	struct KdAccelNode
	{
		void InitLeaf(int *primNums, int np,
			std::vector<int>* primitiveIndices)
		{
			flags = 3;
			nPrims |= (np << 2);
			if (np == 0)
				onePrimitive = 0;
			else if (np == 1)
				onePrimitive = primNums[0];
			else
			{
				primitiveIndicesOffset = primitiveIndices->size();
				for (int i = 0; i < np; ++i)
					primitiveIndices->push_back(primNums[i]);
			}
		}
		void InitInterior(int axis, int ac, float s)
		{
			split = s;
			flags = axis;
			aboveChild |= (ac << 2);
		}
		float SplitPos() const { return split; }
		int nPrimitives() const { return nPrims >> 2; }
		int SplitAxis() const { return flags & 3; }
		bool IsLeaf() const { return (flags & 3) == 3; }
		int AboveChild() const { return aboveChild >> 2; }
		union
		{
			float split;
			int onePrimitive;
			int primitiveIndicesOffset;
		};
		union
		{
			int flags;
			int nPrims;
			int aboveChild;
		};
	};

	enum class EdgeType { Start, End };
	struct BoundEdge {
		// BoundEdge Public Methods
		BoundEdge() {}
		BoundEdge(float t, int primNum, bool starting) : t(t), primNum(primNum) {
			type = starting ? EdgeType::Start : EdgeType::End;
		}
		float t;
		int primNum;
		EdgeType type;
	};

	struct KdToDo {
		const KdAccelNode* node;
		float tMin, tMax;
	};

	KdTreeAccel::KdTreeAccel(const std::vector<std::shared_ptr<Primitive>>& p, int isectCost, int traversalCost,
	                         float emptyBonus, int maxPrims, int maxDepth)
		: isectCost(isectCost), traversalCost(traversalCost),
	maxPrimis(maxPrims), emptyBonus(emptyBonus), primitives(p)
	{
		nextFreeNode = nAllocedNodes = 0;
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * Log2Int(primitives.size()));
		std::vector<Bounds3f> primBounds;
		for(const auto& prim : primitives)
		{
			Bounds3f b = prim->WorldBound();
			bounds.Union(b);
			primBounds.push_back(b);
		}
		std::unique_ptr<BoundEdge[]> edges[3];
		for (int i = 0; i < 3; ++i)
			edges[i].reset(new BoundEdge[2 * primitives.size()]);
		std::unique_ptr<int[]> prims0(new int[primitives.size()]);
		std::unique_ptr<int[]> prims1(new int[(maxDepth + 1) * primitives.size()]);
		std::unique_ptr<int[]> primNums(new int[primitives.size()]);
		for (size_t i = 0; i < primitives.size(); ++i)
			primNums[i] = i;
		buildTree(0, bounds, primBounds, primNums.get(), primitives.size(),
			maxDepth, edges, prims0.get(), prims1.get());
	}

	void KdTreeAccel::buildTree(int nodeNum, const Bounds3f& nodeBounds, const std::vector<Bounds3f>& allPrimBounds,
	                            int* primNums, int nPrimitives, int depth, const std::unique_ptr<BoundEdge[]> edges[3],
	                            int* prims0, int* prims1, int badRefines)
	{
		if(nextFreeNode == nAllocedNodes)
		{
			int nNewAllocNodes = std::max(2 * nAllocedNodes, 512);
			KdAccelNode* n = AllocAligned<KdAccelNode>(nNewAllocNodes);
			if(nAllocedNodes > 0)
			{
				memcpy(n, nodes, nAllocedNodes * sizeof(KdAccelNode));
				FreeAligned(nodes);
			}
			nodes = n;
			nAllocedNodes = nNewAllocNodes;
		}
		++nextFreeNode;
		if (nPrimitives <= maxPrimis || depth == 0)
		{
			nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
			return;
		}
		int bestAxis = -1, bestOffset = -1;
		float bestCost = Infinity;
		float oldCost = isectCost * float(nPrimitives);
		float totalSA = nodeBounds.SurfaceArea();
		float invTotalSA = 1 / totalSA;
		Vector3f d = nodeBounds.pMax - nodeBounds.pMin;
		int axis = nodeBounds.MaximumExtent();
		int retries = 0;
	retrySplit:
		for(int i = 0; i < nPrimitives; ++i)
		{
			int pn = primNums[i];
			const Bounds3f& bounds = allPrimBounds[pn];
			edges[axis][2 * i]     = BoundEdge(bounds.pMin[axis], pn, true);
			edges[axis][2 * i + 1] = BoundEdge(bounds.pMax[axis], pn, false);
		}
		std::sort(&edges[axis][0], &edges[axis][2 * nPrimitives],
			[](const BoundEdge& e0, const BoundEdge& e1)->bool
			{
				if (e0.t == e1.t)
					return (int)e0.type < (int)e1.type;
				else return e0.t < e1.t;
			});
		int nBelow = 0, nAbove = nPrimitives;
		for(int i = 0; i < 2 * nPrimitives; ++i)
		{
			if (edges[axis][i].type == EdgeType::End) --nAbove;
			float edgeT = edges[axis][i].t;
			if (edgeT > nodeBounds.pMin[axis] && edgeT < nodeBounds.pMax[axis])
			{
				int otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;
				float belowSA = 2 * (d[otherAxis0] * d[otherAxis1] +
					(edgeT - nodeBounds.pMin[axis]) * 
					(d[otherAxis0] + d[otherAxis1]));
				float aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] +
					(nodeBounds.pMax[axis] - edgeT) *
					(d[otherAxis0] + d[otherAxis1]));
				float pBelow = belowSA * invTotalSA;
				float pAbove = aboveSA * invTotalSA;
				float eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0;
				float cost =
					traversalCost + (1 - eb) * (pBelow * nBelow + pAbove * nAbove);
				if(cost < bestCost)
				{
					bestCost = cost;
					bestAxis = axis;
					bestOffset = i;
				}
			}
			if (edges[axis][i].type == EdgeType::Start) ++nBelow;
		}
		if(bestAxis == -1 && retries < 2)
		{
			++retries;
			axis = (axis + 1) % 3;
			goto retrySplit;
		}
		if (bestCost > oldCost) ++badRefines;
		if((bestCost > 4*oldCost && nPrimitives < 16) ||
			bestAxis == -1 || badRefines == 3)
		{
			nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
			return;
		}
		int n0 = 0, n1 = 0;
		for (int i = 0; i < bestOffset; ++i)
			if (edges[bestAxis][i].type == EdgeType::Start)
				prims0[n0++] = edges[bestAxis][i].primNum;
		for (int i = bestOffset + 1; i < 2 * nPrimitives; ++i)
			if (edges[bestAxis][i].type == EdgeType::End)
				prims1[n1++] = edges[bestAxis][i].primNum;
		float tSplit = edges[bestAxis][bestOffset].t;
		Bounds3f bounds0 = nodeBounds, bounds1 = nodeBounds;
		bounds0.pMax[bestAxis] = bounds1.pMin[bestAxis] = tSplit;
		buildTree(nodeNum + 1, bounds0, allPrimBounds, prims0, n0,
			depth - 1, edges, prims0, prims1 + nPrimitives, badRefines);
		int aboveChild = nextFreeNode;
		nodes[nodeNum].InitInterior(bestAxis, aboveChild, tSplit);
		buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1,
			depth - 1, edges, prims0, prims1 + nPrimitives, badRefines);
	}

	bool KdTreeAccel::Intersect(const Ray& r, SurfaceInteraction* isect) const
	{
		float tMin, tMax;
		if (!bounds.IntersectP(r, &tMin, &tMax))
			return false;
		Vector3f invDir(1 / r.d.x, 1 / r.d.y, 1 / r.d.z);
		constexpr int maxTodo = 64;
		KdToDo todo[maxTodo];
		int todoPos = 0;

		bool hit = false;
		const KdAccelNode* node = &nodes[0];
		while(node != nullptr)
		{
			if (r.tMax < tMin) break;
			if(!node->IsLeaf())
			{
				int axis = node->SplitAxis();
				float tPlane = (node->SplitPos() - r.o[axis]) * invDir[axis];
				const KdAccelNode* firstChild, * secondChild;
				int belowFrist = (r.o[axis] < node->SplitPos()) ||
					(r.o[axis] == node->SplitPos() && r.d[axis] <= 0);
				if(belowFrist)
				{
					firstChild = node + 1;
					secondChild = &nodes[node->AboveChild()];
				}
				else
				{
					firstChild = &nodes[node->AboveChild()];
					secondChild = node + 1;
				}
				if (tPlane > tMax || tPlane <= 0)
					node = firstChild;
				else if (tPlane < tMin)
					node = secondChild;
				else
				{
					todo[todoPos].node = secondChild;
					todo[todoPos].tMin = tPlane;
					todo[todoPos].tMax = tMax;
					++todoPos;
					node = firstChild;
					tMax = tPlane;
				}
			}
			else
			{
				int nPrimitives = node->nPrimitives();
				if (nPrimitives == 1)
				{
					const std::shared_ptr<Primitive>& p = primitives[node->onePrimitive];
					if (p->Intersect(r, isect))
						hit = true;
				}
				else
				{
					for (int i = 0; i < nPrimitives; ++i)
					{
						int index = primitiveIndices[node->primitiveIndicesOffset + i];
						const std::shared_ptr<Primitive>& p = primitives[index];
						if (p->Intersect(r, isect))
							hit = true;
					}
				}
				if (todoPos > 0) {
					--todoPos;
					node = todo[todoPos].node;
					tMin = todo[todoPos].tMin;
					tMax = todo[todoPos].tMax;
				}
				else
					break;
			}
		}
		return hit;
	}

	bool KdTreeAccel::IntersectP(const Ray& r)
	{
		float tMin, tMax;
		if (!bounds.IntersectP(r, &tMin, &tMax))
			return false;
		Vector3f invDir(1 / r.d.x, 1 / r.d.y, 1 / r.d.z);
		constexpr int maxTodo = 64;
		KdToDo todo[maxTodo];
		int todoPos = 0;
		
		const KdAccelNode* node = &nodes[0];
		while (node != nullptr)
		{
			if (r.tMax < tMin) break;
			if (!node->IsLeaf())
			{
				int axis = node->SplitAxis();
				float tPlane = (node->SplitPos() - r.o[axis]) * invDir[axis];
				const KdAccelNode* firstChild, * secondChild;
				int belowFrist = (r.o[axis] < node->SplitPos()) ||
					(r.o[axis] == node->SplitPos() && r.d[axis] <= 0);
				if (belowFrist)
				{
					firstChild = node + 1;
					secondChild = &nodes[node->AboveChild()];
				}
				else
				{
					firstChild = &nodes[node->AboveChild()];
					secondChild = node + 1;
				}
				if (tPlane > tMax || tPlane <= 0)
					node = firstChild;
				else if (tPlane < tMin)
					node = secondChild;
				else
				{
					todo[todoPos].node = secondChild;
					todo[todoPos].tMin = tPlane;
					todo[todoPos].tMax = tMax;
					++todoPos;
					node = firstChild;
					tMax = tPlane;
				}
			}
			else
			{
				int nPrimitives = node->nPrimitives();
				if (nPrimitives == 1)
				{
					const std::shared_ptr<Primitive>& p = primitives[node->onePrimitive];
					if (p->IntersectP(r))
						return true;
				}
				else
				{
					for (int i = 0; i < nPrimitives; ++i)
					{
						int index = primitiveIndices[node->primitiveIndicesOffset + i];
						const std::shared_ptr<Primitive>& p = primitives[index];
						if (p->IntersectP(r))
							return true;
					}
				}
				if (todoPos > 0) {
					--todoPos;
					node = todo[todoPos].node;
					tMin = todo[todoPos].tMin;
					tMax = todo[todoPos].tMax;
				}
				else
					break;
			}
		}
		return false;
	}


}