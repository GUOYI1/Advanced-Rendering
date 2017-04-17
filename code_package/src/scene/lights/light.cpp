#include "light.h"

Color3f Light::Le(const Ray &r) const
{
    return Color3f(0.f);
}
Point3f Light::GetCenter()
{
    return Point3f(transform.T()*glm::vec4(0,0,0,1));
}
