#include "distancelight.h"
DistanceLight::DistanceLight(const Transform &T,const Color3f &emittedLight,const Vector3f &w)
    : Light(T),emittedLight(emittedLight),wLight(glm::normalize(T.T()*glm::vec4(w,0))) {}

Color3f DistanceLight::Sample_Li(const Base_Intersection &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf, Intersection *LightSample) const {
    *wi = wLight;
    *pdf = 1;
    return emittedLight;

}


Float DistanceLight::Pdf_Li(const Base_Intersection &, const Vector3f &) const {
    return 0;
}
Point3f DistanceLight::GetCenter() const
{
    return Point3f(transform.T()*glm::vec4(0,0,0,1));
}
