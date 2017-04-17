#include "distancelight.h"
DistanceLight::DistanceLight(const Transform &T,const Color3f &emittedLight,const Vector3f &w)
    : Light(T),emittedLight(emittedLight),wLight(glm::normalize(T.T()*glm::vec4(w,0))) {}

Color3f DistanceLight::Sample_Li(const Intersection &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf) const {
    *wi = wLight;
    *pdf = 1;
    return emittedLight;

}


Float DistanceLight::Pdf_Li(const Intersection &, const Vector3f &) const {
    return 0;
}
Point3f DistanceLight::GetCenter()
{
    return Point3f(transform.T()*glm::vec4(0,0,0,1));
}
