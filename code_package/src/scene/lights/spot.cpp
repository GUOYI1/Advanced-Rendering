#include"spot.h"

SpotLight::SpotLight(const Transform &T,const Color3f &emittedLight,
                     Float totalWidth=50.0f, Float falloffStart=20.f)
    : Light(T),pLight(Point3f(T.T()*glm::vec4(Point3f(0, 0, 0),1))),
      emittedLight(emittedLight),
      cosTotalWidth(std::cos(glm::radians(totalWidth))),
      cosFalloffStart(std::cos(glm::radians(falloffStart))) {}

Color3f SpotLight::Sample_Li(const Intersection &ref, const Point2f &u,
                              Vector3f *wi, Float *pdf) const
{
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.f;
    return emittedLight * Falloff(-*wi) / glm::length2(pLight-ref.point);
}

Float SpotLight::Falloff(const Vector3f &w) const {
    Vector3f wl = glm::normalize(Point3f(transform.invT()*glm::vec4(w,0)));
    Float cosTheta = wl.z;
    if (cosTheta < cosTotalWidth) return 0;
    if (cosTheta > cosFalloffStart) return 1;
    // Compute falloff inside spotlight cone
    Float delta = (cosTheta - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);
    return (delta * delta) * (delta * delta);
}


Float SpotLight::Pdf_Li(const Intersection &, const Vector3f &) const {
    return 0.f;
}
Point3f SpotLight::GetCenter()
{
    return pLight;
}

