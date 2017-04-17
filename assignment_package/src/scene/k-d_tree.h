
#pragma once
#include "geometry/primitive.h"


struct KdAccelNode;
struct BoundEdge;
class KdTreeAccel : public Primitive {
  public:
    // KdTreeAccel Public Methods
    KdTreeAccel(const std::vector<std::shared_ptr<Primitive>> &p,
                int maxPrims = 1,int isectCost = 80, int traversalCost = 1,
                Float emptyBonus = 0.5, int maxDepth = -1);
    Bounds3f WorldBound() const { return bounds; }
    ~KdTreeAccel();
    bool Intersect(const Ray &ray, Intersection *isect) const;
    //bool IntersectP(const Ray &ray) const;

  private:
    // KdTreeAccel Private Methods
    void buildTree(int nodeNum, const Bounds3f &bounds,
                   const std::vector<Bounds3f> &primBounds, int *primNums,
                   int nprims, int depth,
                   const std::unique_ptr<BoundEdge[]> edges[3], int *prims0,
                   int *prims1, int badRefines = 0);

    // KdTreeAccel Private Data
    const int isectCost, traversalCost, maxPrims;
    const Float emptyBonus;
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<int> primitiveIndices;
    KdAccelNode *nodes;
    int nAllocedNodes, nextFreeNode;
    Bounds3f bounds;
};

struct KdToDo {
    const KdAccelNode *node;
    Float tMin, tMax;
};

//std::shared_ptr<KdTreeAccel> CreateKdTreeAccelerator(
//    const std::vector<std::shared_ptr<Primitive>> &prims, const ParamSet &ps);




