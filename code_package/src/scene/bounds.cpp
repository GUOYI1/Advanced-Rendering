#include "bounds.h"

bool Bounds3f::Intersect(const Ray &r, float* t) const
{
    //TODO
    Float t0 = -1000000, t1 = 1000000;
    for (int i = 0; i < 3; ++i) {

        Float invRayDir = 1 / r.direction[i];
        Float tNear = (min[i] - r.origin[i]) * invRayDir;
        Float tFar = (max[i] - r.origin[i]) * invRayDir;
        if (tNear > tFar) std::swap(tNear, tFar);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
    }
    if(t0<0)
    {
        Point3f origin=r.origin;
        //Judge whether the origin is inside the bounding box
        if(origin.x<min.x||origin.y<min.y||origin.z||min.z\
                ||origin.x>max.x||origin.y>max.y||origin.z>max.z)
            return false;
        else
            t0=t1;
    }
    if (t) *t = t0;

    return true;
}
//Only used for k-d tree
bool Bounds3f::Intersect(const Ray &r, Float* hitt0,Float* hitt1) const
{
    Float t0 = -std::numeric_limits<Float>::infinity(),
            t1 = std::numeric_limits<Float>::infinity();
    for (int i = 0; i < 3; ++i) {

        Float invRayDir = 1 / r.direction[i];
        Float tNear = (min[i] - r.origin[i]) * invRayDir;
        Float tFar = (max[i] - r.origin[i]) * invRayDir;
        if (tNear > tFar) std::swap(tNear, tFar);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
    }
    if (hitt0) *hitt0 = t0;
    if (hitt1) *hitt1 = t1;
    return true;

}
Bounds3f Bounds3f::Apply(const Transform &tr)
{
    //TODO
    Bounds3f ret(Point3f(tr.T()*glm::vec4(Point3f(min.x, min.y, min.z),1)));
    ret = Union(ret, Point3f(tr.T()*glm::vec4(Point3f(max.x, min.y, min.z),1)));
    ret = Union(ret, Point3f(tr.T()*glm::vec4(Point3f(min.x, max.y, min.z),1)));
    ret = Union(ret, Point3f(tr.T()*glm::vec4(Point3f(min.x, min.y, max.z),1)));
    ret = Union(ret, Point3f(tr.T()*glm::vec4(Point3f(min.x, max.y, max.z),1)));
    ret = Union(ret, Point3f(tr.T()*glm::vec4(Point3f(max.x, max.y, min.z),1)));
    ret = Union(ret, Point3f(tr.T()*glm::vec4(Point3f(max.x, min.y, max.z),1)));
    ret = Union(ret, Point3f(tr.T()*glm::vec4(Point3f(max.x, max.y, max.z),1)));
    return ret;
}

float Bounds3f::SurfaceArea() const
{
    //TODO
    Vector3f d=Diagonal();
    return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

Bounds3f Union(const Bounds3f& b1, const Bounds3f& b2)
{
    return Bounds3f(Point3f(std::min(b1.min.x, b2.min.x),
                            std::min(b1.min.y, b2.min.y),
                            std::min(b1.min.z, b2.min.z)),
                    Point3f(std::max(b1.max.x, b2.max.x),
                            std::max(b1.max.y, b2.max.y),
                            std::max(b1.max.z, b2.max.z)));
}

Bounds3f Union(const Bounds3f& b1, const Point3f& p)
{
    return Bounds3f(Point3f(std::min(b1.min.x, p.x),
                            std::min(b1.min.y, p.y),
                            std::min(b1.min.z, p.z)),
                    Point3f(std::max(b1.max.x, p.x),
                            std::max(b1.max.y, p.y),
                            std::max(b1.max.z, p.z)));
}

Bounds3f Union(const Bounds3f& b1, const glm::vec4& p)
{
    return Union(b1, Point3f(p));
}
