#include "primitive.h"

bool Primitive::Intersect(const Ray &r, Intersection *isect) const
{
    if(!shape->Intersect(r, isect)) return false;
    isect->objectHit = this;
    // We create a BSDF for this intersection in our Integrator classes
    if(material)
    {
        material->ProduceBSDF(isect);
    }
    return true;
}
bool Primitive::IntersectP(const Ray &r) const
{
    Intersection isect;
    if(!shape->Intersect(r, &isect)) return false;

    return true;

}

bool Primitive::ProduceBSDF(Intersection *isect) const
{
    if(material)
    {
        material->ProduceBSDF(isect);
        return true;
    }
    return false;
}
Bounds3f Primitive::WorldBound() const
{
    return shape->WorldBound();
}
const AreaLight* Primitive::GetAreaLight() const
{
    return areaLight.get();
}


const Material* Primitive::GetMaterial() const
{
    return material.get();
}

