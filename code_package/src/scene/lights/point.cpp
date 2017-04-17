#include"point.h"

PointLight::PointLight(const Transform &T,const Color3f &emittedLight)
    : Light(T),pLight(Point3f(T.T()*glm::vec4(Point3f(0, 0, 0),1))),
      emittedLight(emittedLight) {}

Color3f PointLight::Sample_Li(const Intersection &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf) const {
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.f;
    return emittedLight / glm::length2(pLight - ref.point);
}


Float PointLight::Pdf_Li(const Intersection &, const Vector3f &) const {
    return 0;
}

Point3f PointLight::GetCenter()
{
    return pLight;
}
