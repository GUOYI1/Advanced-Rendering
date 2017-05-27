#include <raytracing/intersection.h>
Base_Intersection::Base_Intersection():
    point(Point3f(0)),
    normalGeometric(Normal3f(0)),t(-1)
{}

Ray Base_Intersection::SpawnRay(const Vector3f &d) const
{
    Vector3f originOffset = normalGeometric * RayEpsilon;
    // Make sure to flip the direction of the offset so it's in
    // the same general direction as the ray direction
    originOffset = (glm::dot(d, normalGeometric) > 0) ? originOffset : -originOffset;
    Point3f o(this->point + originOffset);
    return Ray(o, d,Infinity,GetMedium(d));
}
Ray Base_Intersection::SpawnRayTo(const Intersection &it) const
{
    Vector3f originOffset = normalGeometric * RayEpsilon;
    Vector3f d1 = it.point - point;
    originOffset = (glm::dot(d1, normalGeometric) > 0) ? originOffset : -originOffset;
    Point3f o(this->point + originOffset);

    Vector3f targetOffset=it.normalGeometric*RayEpsilon;
    Vector3f d2 = o-it.point;
    targetOffset = (glm::dot(d2, it.normalGeometric) > 0) ? targetOffset : -targetOffset;
    Point3f t(it.point+targetOffset);

    Vector3f d=t-o;

    return Ray(o,d,1.0-0.00001f,GetMedium(d));
}
Intersection::Intersection():
    Base_Intersection(),
    uv(Point2f(0)),
    objectHit(nullptr),
    bsdf(nullptr),
    tangent(0.f), bitangent(0.f)
{}

bool Intersection::ProduceBSDF()
{
    return objectHit->ProduceBSDF(this);
}

Color3f Intersection::Le(const Vector3f &wo) const
{
    const AreaLight* light = objectHit->GetAreaLight();
    return light ? light->L(Base_Intersection(*this), wo) : Color3f(0.f);
}


