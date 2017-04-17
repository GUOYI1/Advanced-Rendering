#ifndef INVERSESPHERE
#define INVERSESPHERE

#endif // INVERSESPHERE
#include <scene/geometry/sphere.h>
class InverseSphere : public Sphere
{
public:
    virtual void ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const;
};


