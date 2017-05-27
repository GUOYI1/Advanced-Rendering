#include "bvh.h"
#include<iostream>
#include<QTime>
// Feel free to ignore these structs entirely!
// They are here if you want to implement any of PBRT's
// methods for BVH construction.

struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() {}
    BVHPrimitiveInfo(size_t primitiveNumber, const Bounds3f &bounds)
        : primitiveNumber(primitiveNumber),
          bounds(bounds),
          centroid(.5f * bounds.min + .5f * bounds.max) {}
    int primitiveNumber;
    Bounds3f bounds;
    Point3f centroid;
};
//struct CompareToMid {
//    CompareToMid(int d, float m) { dim = d; mid = m; }
//    int dim;
//    float mid;
//    bool operator()(const BVHPrimitiveInfo &a) const {
//        return a.centroid[dim] < mid;
//    }
//};

struct ComparePoints {
    ComparePoints(int d) { dim = d; }
    int dim;
    bool operator()(const BVHPrimitiveInfo &a,
                    const BVHPrimitiveInfo &b) const {
        return a.centroid[dim] < b.centroid[dim];
    }
};

struct CompareToBucket {
    CompareToBucket(int split, int num, int d, const Bounds3f &b)
        : centroidBounds(b)
    { splitBucket = split; nBuckets = num; dim = d; }
    bool operator()(const BVHPrimitiveInfo &p) const;

    int splitBucket, nBuckets, dim;
    const Bounds3f &centroidBounds;
};
bool CompareToBucket::operator()(const BVHPrimitiveInfo &p) const
{
     int b = nBuckets * ((p.centroid[dim] - centroidBounds.min[dim]) /
             (centroidBounds.max[dim] - centroidBounds.min[dim]));
     if (b == nBuckets) b = nBuckets-1;
     return b <= splitBucket;
}


struct BVHBuildNode {
    // BVHBuildNode Public Methods
    void InitLeaf(int first, int n, const Bounds3f &b) {
        firstPrimOffset = first;
        Primitive_num = n;
        bounds = b;
        children[0] = children[1] = nullptr;
    }
    void InitInterior(int axis, BVHBuildNode *c0, BVHBuildNode *c1) {
        children[0] = c0;
        children[1] = c1;
        bounds = Union(c0->bounds, c1->bounds);
        splitAxis = axis;
        Primitive_num = 0;
    }
    Bounds3f bounds;
    BVHBuildNode *children[2];
    int splitAxis, firstPrimOffset, Primitive_num;
};

struct MortonPrimitive {
    int primitiveIndex;
    unsigned int mortonCode;
};

struct LBVHTreelet {
    int startIndex, Primitive_num;
    BVHBuildNode *buildNodes;
};

struct LinearBVHNode {
    Bounds3f bounds;
    union {
        int primitivesOffset;   // leaf
        int secondChildOffset;  // interior
    };
    unsigned short Primitive_num;  // 0 -> interior node, 16 bytes
    unsigned char axis;          // interior node: xyz, 8 bytes
    unsigned char pad[1];        // ensure 32 byte total size
};


BVHAccel::~BVHAccel()
{
    delete [] nodes;
}

// Constructs an array of BVHPrimitiveInfos, recursively builds a node-based BVH
// from the information, then optimizes the memory of the BVH
BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive> > &p, int maxPrimsInNode)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), primitives(p)
{
    //TODO
    int t=0;
    QTime timer;
    timer.start();
    if (primitives.size() == 0)
        return;
    std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
    for (size_t i = 0; i < primitives.size(); ++i)
        primitiveInfo[i] = {i, primitives[i]->WorldBound() };

    //Build BVH tree for primitives using primitiveInfo
    int totalNodes = 0;
    std::vector<std::shared_ptr<Primitive>> Prims_list;
    BVHBuildNode *root;

    root = recursiveBuild(primitiveInfo, 0, primitives.size(), &totalNodes, Prims_list);
    primitives.swap(Prims_list);


    nodes = new LinearBVHNode[totalNodes];
    int offset = 0;
    flattenBVHTree(root, &offset);
    t=timer.elapsed();
    std::cout<<"BVH Construction Time:"<<t<<std::endl;
}

bool BVHAccel::Intersect(const Ray &ray, Intersection *isect) const
{
    //TODO
    bool hit = false;
    Vector3f invDir(1 / ray.direction.x, 1 / ray.direction.y, 1 / ray.direction.z);
    int dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };
    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[64];
    float t_min=std::numeric_limits<Float>::infinity();
    Intersection isect_min;
    while (true)
    {
        const LinearBVHNode *node = &nodes[currentNodeIndex];
//        float* temp_t;
//        if(node->bounds.Intersect(ray,temp_t))
        if (node->bounds.IntersectP(ray, invDir, dirIsNeg))
        {
            if (node->Primitive_num > 0)
            {
                //Intersect ray with primitives in leaf BVH node
                for (int i = 0; i < node->Primitive_num; ++i)
                    if (primitives[node->primitivesOffset + i]->Intersect(ray, isect))
                    {
                        hit = true;
                        if(isect->t>0&&isect->t<t_min)
                        {
                            t_min=isect->t;
                            isect_min=*isect;
                        }
                    }
                if (toVisitOffset == 0) break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
            else
            {
                //Put far BVH node on nodesToVisit stack, advance to near node
                if (dirIsNeg[node->axis])
                {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                }
                else
                {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
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
    if(isect_min.t>ray.tMax)
        return false;
    *isect=isect_min;
    ray.tMax=isect->t;
    return hit;
}

BVHBuildNode *BVHAccel::recursiveBuild(
    std::vector<BVHPrimitiveInfo> &primitiveInfo,
    int start, int end, int *totalNodes,
    std::vector<std::shared_ptr<Primitive>> &Prims_list)
{
    BVHBuildNode *node=new BVHBuildNode;
    (*totalNodes)++;
    Bounds3f bounds;
    for (int i = start; i < end; ++i)
        bounds = Union(bounds, primitiveInfo[i].bounds);
    int Primitive_num = end - start;
    if (Primitive_num == 1)
    {
        int firstPrimOffset = Prims_list.size();
        for (int i = start; i < end; ++i)
        {
            int primNum = primitiveInfo[i].primitiveNumber;
            Prims_list.push_back(primitives[primNum]);
        }
        node->InitLeaf(firstPrimOffset, Primitive_num, bounds);
        return node;
    }
    else
    {
        //Compute bound of primitive centroids, choose split dimension dim
        Bounds3f centroidBounds=Bounds3f(primitiveInfo[start].centroid);
        for (int i = start; i < end; ++i)
            centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
        int dim = centroidBounds.MaximumExtent();

        //Partition primitives into two sets and build children
        int mid = (start + end) / 2;
        if (centroidBounds.max[dim] == centroidBounds.min[dim])
        {
            int firstPrimOffset = Prims_list.size();
            for (int i = start; i < end; ++i) {
            int primNum = primitiveInfo[i].primitiveNumber;
            Prims_list.push_back(primitives[primNum]);
            }
            node->InitLeaf(firstPrimOffset, Primitive_num, bounds);
            return node;
        }
        else
        {
        //Partition primitives based on splitMethod

            //Partition primitives using approximate SAH
            // Partition primitives using approximate SAH
            if (Primitive_num <= 4) {
                // Partition primitives into equally-sized subsets
                mid = (start + end) / 2;
                std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
                                 &primitiveInfo[end-1]+1, ComparePoints(dim));
            }
            else {
                // Allocate _BucketInfo_ for SAH partition buckets
                const int nBuckets = 12;
                struct BucketInfo
                {
                    BucketInfo() { count = 0; }
                    int count;
                    Bounds3f bounds;
                };
                BucketInfo buckets[nBuckets];

                // Initialize _BucketInfo_ for SAH partition buckets
                for (int i = start; i < end; ++i) {
                    int b = nBuckets *
                            ((primitiveInfo[i].centroid[dim] - centroidBounds.min[dim]) /
                             (centroidBounds.max[dim] - centroidBounds.min[dim]));
                    if (b == nBuckets) b = nBuckets-1;
                    //Assert(b >= 0 && b < nBuckets);
                    buckets[b].count++;
                    buckets[b].bounds = Union(buckets[b].bounds, primitiveInfo[i].bounds);
                }

                // Compute costs for splitting after each bucket
                float cost[nBuckets-1];
                for (int i = 0; i < nBuckets-1; ++i) {
                    Bounds3f b0, b1;
                    int count0 = 0, count1 = 0;
                    for (int j = 0; j <= i; ++j) {
                        b0 = Union(b0, buckets[j].bounds);
                        count0 += buckets[j].count;
                    }
                    for (int j = i+1; j < nBuckets; ++j) {
                        b1 = Union(b1, buckets[j].bounds);
                        count1 += buckets[j].count;
                    }
                    cost[i] = .125f + (count0*b0.SurfaceArea() + count1*b1.SurfaceArea()) /
                            bounds.SurfaceArea();
                }

                // Find bucket to split at that minimizes SAH metric
                float minCost = cost[0];
                int minCostSplit = 0;
                for (int i = 1; i < nBuckets-1; ++i) {
                    if (cost[i] < minCost) {
                        minCost = cost[i];
                        minCostSplit = i;
                    }
                }

                // Either create leaf or split primitives at selected SAH bucket
                if (Primitive_num > maxPrimsInNode ||
                        minCost < Primitive_num) {
                    BVHPrimitiveInfo *pmid = std::partition(&primitiveInfo[start],
                                                            &primitiveInfo[end-1]+1,
                            CompareToBucket(minCostSplit, nBuckets, dim, centroidBounds));
                    mid = pmid - &primitiveInfo[0];
                }

                else {
                    // Create leaf _BVHBuildNode_
                    int firstPrimOffset = Prims_list.size();
                    for (int i = start; i < end; ++i) {
                        int primNum = primitiveInfo[i].primitiveNumber;
                        Prims_list.push_back(primitives[primNum]);
                    }
                    node->InitLeaf(firstPrimOffset, Primitive_num, bounds);
                }
            }
            node->InitInterior(dim,recursiveBuild(primitiveInfo, start, mid,totalNodes, Prims_list),
                               recursiveBuild(primitiveInfo, mid, end, totalNodes, Prims_list));
        }
    }
    return node;
}
int BVHAccel::flattenBVHTree(BVHBuildNode *node, int *offset)
{
    LinearBVHNode *linearNode = &nodes[*offset];
    linearNode->bounds = node->bounds;
    int myOffset = (*offset)++;
    if (node->Primitive_num > 0)
    {
        linearNode->primitivesOffset = node->firstPrimOffset;
        linearNode->Primitive_num = node->Primitive_num;
    }
    else
    {
        //Create interior flattened BVH node
        linearNode->axis = node->splitAxis;
        linearNode->Primitive_num = 0;
        flattenBVHTree(node->children[0], offset);
        linearNode->secondChildOffset = flattenBVHTree(node->children[1], offset);
    }
    return myOffset;
}
// We don't need this
BVHBuildNode *BVHAccel::buildUpperSAH(std::vector<BVHBuildNode *> &treeletRoots,
                                      int start, int end,
                                      int *totalNodes) const
{
    int nNodes = end - start;
    if (nNodes == 1) return treeletRoots[start];
    (*totalNodes)++;
    BVHBuildNode *node = new BVHBuildNode;

    // Compute bounds of all nodes under this HLBVH node
    Bounds3f bounds;
    for (int i = start; i < end; ++i)
        bounds = Union(bounds, treeletRoots[i]->bounds);

    // Compute bound of HLBVH node centroids, choose split dimension _dim_
    Bounds3f centroidBounds;
    for (int i = start; i < end; ++i) {
        Point3f centroid =
            (treeletRoots[i]->bounds.min + treeletRoots[i]->bounds.max) *
            0.5f;
        centroidBounds = Union(centroidBounds, centroid);
    }
    int dim = centroidBounds.MaximumExtent();

    // Allocate _BucketInfo_ for SAH partition buckets
    int nBuckets = 12;
    struct BucketInfo {
        int count = 0;
        Bounds3f bounds;
    };
    BucketInfo buckets[nBuckets];

    // Initialize _BucketInfo_ for HLBVH SAH partition buckets
    for (int i = start; i < end; ++i) {
        Float centroid = (treeletRoots[i]->bounds.min[dim] +
                          treeletRoots[i]->bounds.max[dim]) *
                         0.5f;
        int b =
            nBuckets * ((centroid - centroidBounds.min[dim]) /
                        (centroidBounds.max[dim] - centroidBounds.min[dim]));
        if (b == nBuckets) b = nBuckets - 1;
        buckets[b].count++;
        buckets[b].bounds = Union(buckets[b].bounds, treeletRoots[i]->bounds);
    }

    // Compute costs for splitting after each bucket
    Float cost[nBuckets - 1];
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
        cost[i] = .125f +
                  (count0 * b0.SurfaceArea() + count1 * b1.SurfaceArea()) /
                      bounds.SurfaceArea();
    }

    // Find bucket to split at that minimizes SAH metric
    Float minCost = cost[0];
    int minCostSplitBucket = 0;
    for (int i = 1; i < nBuckets - 1; ++i) {
        if (cost[i] < minCost) {
            minCost = cost[i];
            minCostSplitBucket = i;
        }
    }

    // Split nodes and create interior HLBVH SAH node
    BVHBuildNode **pmid = std::partition(
        &treeletRoots[start], &treeletRoots[end - 1] + 1,
        [=](const BVHBuildNode *node) {
            Float centroid =
                (node->bounds.min[dim] + node->bounds.max[dim]) * 0.5f;
            int b = nBuckets *
                    ((centroid - centroidBounds.min[dim]) /
                     (centroidBounds.max[dim] - centroidBounds.min[dim]));
            if (b == nBuckets) b = nBuckets - 1;
            return b <= minCostSplitBucket;
        });
    int mid = pmid - &treeletRoots[0];
    node->InitInterior(
        dim, this->buildUpperSAH(treeletRoots, start, mid, totalNodes),
        this->buildUpperSAH(treeletRoots, mid, end, totalNodes));
    return node;
}
