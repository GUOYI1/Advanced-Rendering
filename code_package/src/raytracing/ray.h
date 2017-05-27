#pragma once
#include <globals.h>
class Medium;
class Ray
{
public:
    Ray(const Point3f &o, const Vector3f &d);
    Ray(const glm::vec4 &o, const glm::vec4 &d);

    Ray(const Ray &r);
    Ray(const Point3f &o, const Vector3f &d, Float tMax ,const Medium *medium=nullptr);
    //Return a copy of this ray that has been transformed
    //by the input transformation matrix.
    Ray GetTransformedCopy(const Matrix4x4 &T) const;

    Point3f origin;
    Vector3f direction;
    Point3f operator()(Float ft) const {return origin+direction*ft;}
    mutable Float tMax;
    //Float time;
    const Medium *medium;
};
