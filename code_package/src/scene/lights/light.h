#pragma once
#include <globals.h>
#include <scene/transform.h>
#include <raytracing/intersection.h>
enum LightType{
    Area=1<<0,
    Point=1<<1,
    Spot=1<<2,
    Distance=1<<3,
    Infinite=1<<4
};
inline bool IsDeltaLight(LightType type) {
    return type & (int)LightType::Point ||
           type & (int)LightType::Spot||
           type & (int)LightType::Distance;
}
class Base_Intersection;
class Intersection;
class Light
{
  public:
    virtual ~Light(){}
    Light(Transform t)
        : transform(t), name()
    {}

    // Returns the light emitted along a ray that does
    // not hit anything within the scene bounds.
    // Necessary if we want to support things like environment
    // maps, or other sources of light with infinite area.
    // The default implementation for general lights returns
    // no energy at all.
    virtual Color3f Le(const Ray &r) const;

    virtual Color3f Sample_Li(const Base_Intersection &ref, const Point2f &xi,
                                                Vector3f *wi, Float *pdf,Intersection* LightSample=nullptr) const = 0;

    virtual float Pdf_Li(const Base_Intersection &ref, const Vector3f &wiW) const=0;
    virtual Point3f GetCenter() const;
    QString name; // For debugging
    LightType type;

 protected:
    const Transform transform;

};

class AreaLight : public Light
{
public:
    AreaLight(const Transform &t) : Light(t){}
    // Returns the light emitted from a point on the light's surface _isect_
    // along the direction _w_, which is leaving the surface.
    virtual Color3f L(const Base_Intersection &isect, const Vector3f &w) const = 0;
};


