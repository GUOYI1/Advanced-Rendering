#include "k-d_tree.h"
#include<iostream>
#include<QTime>
struct KdAccelNode {
    // KdAccelNode Methods
    void InitLeaf(int *primNums, int np, std::vector<int> *primitiveIndices)
    {
       flags = 3;
       nPrims |= (np << 2);
       // Store primitive ids for leaf node
       if (np == 0)
           onePrimitive = 0;
       else if (np == 1)
           onePrimitive = primNums[0];
       else {
           primitiveIndicesOffset = primitiveIndices->size();
           for (int i = 0; i < np; ++i)
               primitiveIndices->push_back(primNums[i]);
       }
    }
    void InitInterior(int axis, int ac, Float s) {
        split = s;
        flags = axis;
        aboveChild |= (ac << 2);
    }
    Float SplitPos() const { return split; }
    int nPrimitives() const { return nPrims >> 2; }
    int SplitAxis() const { return flags & 3; }
    bool IsLeaf() const { return (flags & 3) == 3; }
    int AboveChild() const { return aboveChild >> 2; }
    union {
        Float split;                 // Interior
        int onePrimitive;            // Leaf
        int primitiveIndicesOffset;  // Leaf
    };

    private:
    union {
        int flags;       // Both
        int nPrims;      // Leaf
        int aboveChild;  // Interior
    };
};

enum class EdgeType { Start, End };
struct BoundEdge {
    // BoundEdge Public Methods
    BoundEdge() {}
    BoundEdge(Float t, int primNum, bool starting) : t(t), primNum(primNum) {
        type = starting ? EdgeType::Start : EdgeType::End;
    }
    Float t;
    int primNum;
    EdgeType type;
};

// KdTreeAccel Method Definitions
KdTreeAccel::KdTreeAccel(const std::vector<std::shared_ptr<Primitive>> &p,
                         int maxPrims,int isectCost, int traversalCost, Float emptyBonus, int maxDepth)
    : isectCost(isectCost),
      traversalCost(traversalCost),
      maxPrims(maxPrims),
      emptyBonus(emptyBonus),
      primitives(p) {

    int t=0;
    QTime timer;
    timer.start();

    nextFreeNode = nAllocedNodes = 0;
    if (maxDepth <= 0)
        maxDepth = std::round(8 + 1.3f * Log2Int(int64_t(primitives.size())));

    // Compute bounds for kd-tree construction
    std::vector<Bounds3f> primBounds;
    primBounds.reserve(primitives.size());
    for (const std::shared_ptr<Primitive> &prim : primitives) {
        Bounds3f b = prim->WorldBound();
        bounds = Union(bounds, b);
        primBounds.push_back(b);
    }

    // Allocate working memory for kd-tree construction
    std::unique_ptr<BoundEdge[]> edges[3];
    for (int i = 0; i < 3; ++i)
        edges[i].reset(new BoundEdge[2 * primitives.size()]);
    std::unique_ptr<int[]> prims0(new int[primitives.size()]);
    std::unique_ptr<int[]> prims1(new int[(maxDepth + 1) * primitives.size()]);

    // Initialize _primNums_ for kd-tree construction
    std::unique_ptr<int[]> primNums(new int[primitives.size()]);
    for (size_t i = 0; i < primitives.size(); ++i) primNums[i] = i;

    // Start recursive construction of kd-tree
    buildTree(0, bounds, primBounds, primNums.get(), primitives.size(),
              maxDepth, edges, prims0.get(), prims1.get());

    t=timer.elapsed();
    std::cout<<"KD Tree Construction Time:"<<t<<std::endl;
}



void KdTreeAccel::buildTree(int nodeNum, const Bounds3f &nodeBounds,
                            const std::vector<Bounds3f> &allPrimBounds,
                            int *primNums, int nPrimitives, int depth,
                            const std::unique_ptr<BoundEdge[]> edges[3],
                            int *prims0, int *prims1, int badRefines) {
    // Get next free node from _nodes_ array
    if (nextFreeNode == nAllocedNodes) {
        int nNewAllocNodes = std::max(2 * nAllocedNodes, 512);
        KdAccelNode *n =  new KdAccelNode[nNewAllocNodes];
        if (nAllocedNodes > 0)
        {
            memcpy(n, nodes, nAllocedNodes * sizeof(KdAccelNode));
            if(nodes)
                free(nodes);
        }
        nodes = n;
        nAllocedNodes = nNewAllocNodes;
    }
    ++nextFreeNode;

    // Initialize leaf node if termination criteria met
    if (nPrimitives <= maxPrims || depth == 0) {
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    // Initialize interior node and continue recursion

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    Float bestCost = std::numeric_limits<Float>::infinity();
    Float oldCost = isectCost * Float(nPrimitives);
    Float totalSA = nodeBounds.SurfaceArea();
    Float invTotalSA = 1 / totalSA;
    Vector3f d = nodeBounds.max - nodeBounds.min;

    // Choose which axis to split along
    int axis = nodeBounds.MaximumExtent();
    int retries = 0;
retrySplit:

    // Initialize edges for _axis_
    for (int i = 0; i < nPrimitives; ++i) {
        int pn = primNums[i];
        const Bounds3f &bounds = allPrimBounds[pn];
        edges[axis][2 * i] = BoundEdge(bounds.min[axis], pn, true);
        edges[axis][2 * i + 1] = BoundEdge(bounds.max[axis], pn, false);
    }

    // Sort _edges_ for _axis_
    std::sort(&edges[axis][0], &edges[axis][2 * nPrimitives],
              [](const BoundEdge &e0, const BoundEdge &e1) -> bool {
                  if (e0.t == e1.t)
                      return (int)e0.type < (int)e1.type;
                  else
                      return e0.t < e1.t;
              });

    // Compute cost of all splits for _axis_ to find best
    int nBelow = 0, nAbove = nPrimitives;
    for (int i = 0; i < 2 * nPrimitives; ++i) {
        if (edges[axis][i].type == EdgeType::End) --nAbove;
        Float edgeT = edges[axis][i].t;
        if (edgeT > nodeBounds.min[axis] && edgeT < nodeBounds.max[axis]) {
            // Compute cost for split at _i_th edge

            // Compute child surface areas for split at _edgeT_
            int otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;
            Float belowSA = 2 * (d[otherAxis0] * d[otherAxis1] + (edgeT - nodeBounds.min[axis]) *
                                     (d[otherAxis0] + d[otherAxis1]));
            Float aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] + (nodeBounds.max[axis] - edgeT) *
                                     (d[otherAxis0] + d[otherAxis1]));
            Float pBelow = belowSA * invTotalSA;
            Float pAbove = aboveSA * invTotalSA;
            Float eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0;
            Float cost =traversalCost + isectCost * (1 - eb) * (pBelow * nBelow + pAbove * nAbove);

            // Update best split if this is lowest cost so far
            if (cost < bestCost) {
                bestCost = cost;
                bestAxis = axis;
                bestOffset = i;
            }
        }
        if (edges[axis][i].type == EdgeType::Start) ++nBelow;
    }

    // Create leaf if no good splits were found
    if (bestAxis == -1 && retries < 2)
    {
        ++retries;
        axis = (axis + 1) % 3;
        goto retrySplit;
    }
    if (bestCost > oldCost)
        ++badRefines;
    if ((bestCost > 4 * oldCost && nPrimitives < 16) || bestAxis == -1 ||
        badRefines == 3)
    {
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    // Classify primitives with respect to split
    int n0 = 0, n1 = 0;
    for (int i = 0; i < bestOffset; ++i)
        if (edges[bestAxis][i].type == EdgeType::Start)
            prims0[n0++] = edges[bestAxis][i].primNum;
    for (int i = bestOffset + 1; i < 2 * nPrimitives; ++i)
        if (edges[bestAxis][i].type == EdgeType::End)
            prims1[n1++] = edges[bestAxis][i].primNum;

    // Recursively initialize children nodes
    Float tSplit = edges[bestAxis][bestOffset].t;
    Bounds3f bounds0 = nodeBounds, bounds1 = nodeBounds;
    bounds0.max[bestAxis] = bounds1.min[bestAxis] = tSplit;
    buildTree(nodeNum + 1, bounds0, allPrimBounds, prims0, n0, depth - 1, edges,
              prims0, prims1 + nPrimitives, badRefines);
    int aboveChild = nextFreeNode;
    nodes[nodeNum].InitInterior(bestAxis, aboveChild, tSplit);
    buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1, depth - 1, edges,
              prims0, prims1 + nPrimitives, badRefines);
}

bool KdTreeAccel::Intersect(const Ray &ray, Intersection *isect) const {
    // Compute initial parametric range of ray inside kd-tree extent
    Float tMin, tMax;

    if (!bounds.Intersect(ray, &tMin, &tMax)) {
        return false;
    }

    // Prepare to traverse kd-tree for ray
    Vector3f invDir(1 / ray.direction.x, 1 / ray.direction.y, 1 / ray.direction.z);
    int maxTodo = 64;
    KdToDo todo[maxTodo];
    int todoPos = 0;

    // Traverse kd-tree nodes in order for ray
    bool hit = false;
    const KdAccelNode *node = &nodes[0];
    Float t_min_hit=std::numeric_limits<Float>::infinity();
    Intersection isect_min;
    while (node != nullptr) {
        // Bail out if we found a hit closer than the current node
        //if (ray.tMax < tMin) break;
        if (!node->IsLeaf()) {
            // Process kd-tree interior node

            // Compute parametric distance along ray to split plane
            int axis = node->SplitAxis();
            Float tPlane = (node->SplitPos() - ray.origin[axis]) * invDir[axis];

            // Get node children pointers for ray
            const KdAccelNode *firstChild, *secondChild;
            int belowFirst =(ray.origin[axis] < node->SplitPos()) ||
                (ray.origin[axis] == node->SplitPos() && ray.direction[axis] <= 0);
            if (belowFirst)
            {
                firstChild = node + 1;
                secondChild = &nodes[node->AboveChild()];
            }
            else {
                firstChild = &nodes[node->AboveChild()];
                secondChild = node + 1;
            }

            // Advance to next child node, possibly enqueue other child
            if (tPlane > tMax || tPlane <= 0)
                node = firstChild;
            else if (tPlane < tMin)
                node = secondChild;
            else {
                // Enqueue _secondChild_ in todo list
                todo[todoPos].node = secondChild;
                todo[todoPos].tMin = tPlane;
                todo[todoPos].tMax = tMax;
                ++todoPos;
                node = firstChild;
                tMax = tPlane;
            }
        } else {
            // Check for intersections inside leaf node
            int nPrimitives = node->nPrimitives();
            if (nPrimitives == 1) {
                const std::shared_ptr<Primitive> &p = primitives[node->onePrimitive];
                // Check one primitive inside leaf node
                if (p->Intersect(ray, isect))
                {
                    hit = true;
                    if(isect->t>0 && isect->t<t_min_hit)
                    {
                        t_min_hit=isect->t;
                        isect_min=*isect;
                    }
                }
            } else {
                for (int i = 0; i < nPrimitives; ++i) {
                    int index =primitiveIndices[node->primitiveIndicesOffset + i];
                    const std::shared_ptr<Primitive> &p = primitives[index];
                    // Check one primitive inside leaf node
                    if (p->Intersect(ray, isect))
                    {
                        hit = true;
                        if(isect->t>0 && isect->t<t_min_hit)
                        {
                            t_min_hit=isect->t;
                            isect_min=*isect;
                        }
                    }
                }
            }

            // Grab next node to process from todo list
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
    *isect=isect_min;
    return hit;
}

//We don't use this

//bool KdTreeAccel::IntersectP(const Ray &ray) const {
//    // Compute initial parametric range of ray inside kd-tree extent
//    Float tMin, tMax;
//    if (!bounds.Intersect(ray, &tMin, &tMax)) {
//        return false;
//    }

//    // Prepare to traverse kd-tree for ray
//    Vector3f invDir(1 / ray.direction.x, 1 / ray.direction.y, 1 / ray.direction.z);
//    int maxTodo = 64;
//    KdToDo todo[maxTodo];
//    int todoPos = 0;
//    const KdAccelNode *node = &nodes[0];
//    while (node != nullptr) {
//        if (node->IsLeaf()) {
//            // Check for shadow ray intersections inside leaf node
//            int nPrimitives = node->nPrimitives();
//            if (nPrimitives == 1) {
//                const std::shared_ptr<Primitive> &p =
//                    primitives[node->onePrimitive];
//                if (p->IntersectP(ray)) {
//                    return true;
//                }
//            } else {
//                for (int i = 0; i < nPrimitives; ++i) {
//                    int primitiveIndex =
//                        primitiveIndices[node->primitiveIndicesOffset + i];
//                    const std::shared_ptr<Primitive> &prim =
//                        primitives[primitiveIndex];
//                    if (prim->IntersectP(ray)) {
//                        return true;
//                    }
//                }
//            }

//            // Grab next node to process from todo list
//            if (todoPos > 0) {
//                --todoPos;
//                node = todo[todoPos].node;
//                tMin = todo[todoPos].tMin;
//                tMax = todo[todoPos].tMax;
//            } else
//                break;
//        } else {
//            // Process kd-tree interior node

//            // Compute parametric distance along ray to split plane
//            int axis = node->SplitAxis();
//            Float tPlane = (node->SplitPos() - ray.origin[axis]) * invDir[axis];

//            // Get node children pointers for ray
//            const KdAccelNode *firstChild, *secondChild;
//            int belowFirst =
//                (ray.origin[axis] < node->SplitPos()) ||
//                (ray.origin[axis] == node->SplitPos() && ray.direction[axis] <= 0);
//            if (belowFirst) {
//                firstChild = node + 1;
//                secondChild = &nodes[node->AboveChild()];
//            } else {
//                firstChild = &nodes[node->AboveChild()];
//                secondChild = node + 1;
//            }

//            // Advance to next child node, possibly enqueue other child
//            if (tPlane > tMax || tPlane <= 0)
//                node = firstChild;
//            else if (tPlane < tMin)
//                node = secondChild;
//            else {
//                // Enqueue _secondChild_ in todo list
//                todo[todoPos].node = secondChild;
//                todo[todoPos].tMin = tPlane;
//                todo[todoPos].tMax = tMax;
//                ++todoPos;
//                node = firstChild;
//                tMax = tPlane;
//            }
//        }
//    }
//    return false;
//}
KdTreeAccel::~KdTreeAccel()
{
   delete [] nodes;
}
