#pragma once
#include <globals.h>
#include <scene/materials/bsdf.h>
#include <raytracing/ray.h>
#include <scene/geometry/primitive.h>
#include <QList>
#include<scene/media/media.h>
class Material;
class BSDF;
class Primitive;

class Intersection;
class Base_Intersection
{
public:
    Base_Intersection();
    Base_Intersection(const Point3f &p, const Normal3f &n, Float t,
                   const MediumInterface &mediumInterface)
           : point(p),
             t(t),
             normalGeometric(n),
             mediumInterface(mediumInterface){}
    Base_Intersection(const Point3f &p, Float t,const MediumInterface &mediumInterface): \
        point(p), t(t),  normalGeometric(Normal3f(0.0,0.0,0.0)),mediumInterface(mediumInterface) {}
    bool IsSurfaceInteraction() const { return normalGeometric != Normal3f(); }
    bool IsMediumInteraction() const { return !IsSurfaceInteraction(); }
    Ray SpawnRay(const Vector3f &d) const;
    Ray SpawnRayTo(const Intersection &it) const;
    const Medium *GetMedium(const Vector3f &w) const
    {
            return glm::dot(w, normalGeometric) > 0 ? mediumInterface.outside : mediumInterface.inside;
    }
    const Medium *GetMedium() const
    {
            return mediumInterface.inside;
    }
    Point3f point;          // The place at which the intersection occurred
    Normal3f normalGeometric; // The surface normal at the point of intersection, NO alterations like normal mapping applied
    float t;               // The parameterization for the ray (in world space) that generated this intersection.
    MediumInterface mediumInterface;
};

class Intersection:public Base_Intersection
{
public:
    Intersection();
    Intersection(const Point3f &p, const Normal3f &n, Float t,
                   const MediumInterface &mediumInterface)
           : Base_Intersection(p,n,t,mediumInterface){}
    Intersection(const Point3f &p, Float t,const MediumInterface &mediumInterface): \
        Base_Intersection(p,t,mediumInterface){}
    // Ask _objectHit_ to produce a BSDF
    // based on other data stored in this
    // Intersection, e.g. the surface normal
    // and UV coordinates
    bool ProduceBSDF();

    // Returns the light emitted from this Intersection
    // along _wo_, provided that this Intersection is
    // on the surface of an AreaLight. If not, then
    // zero light is returned.
    Color3f Le(const Vector3f& wo) const;

    // Instantiate a Ray that originates from this Intersection and
    // travels in direction d.





    Point2f uv;             // The UV coordinates computed at the intersection
    Primitive const * objectHit;     // The object that the ray intersected, or nullptr if the ray hit nothing.
    std::shared_ptr<BSDF> bsdf;// The Bidirection Scattering Distribution Function found at the intersection.

    Vector3f tangent, bitangent; // World-space vectors that form an orthonormal basis with the surface normal.

};
class MediumInteraction : public Base_Intersection {
  public:
    // MediumInteraction Public Methods
    MediumInteraction() : phase(nullptr) {}
    MediumInteraction(const Point3f &p, Float t, const Medium *medium, std::shared_ptr<PhaseFunction> phase)
        : Base_Intersection(p, t, medium), phase(phase) {}
    bool IsValid() const { return phase != nullptr; }

    // MediumInteraction Public Data
    std::shared_ptr<PhaseFunction> phase;

};

