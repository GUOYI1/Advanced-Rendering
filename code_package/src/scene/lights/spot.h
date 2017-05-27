#pragma once

#include "light.h"

class SpotLight : public Light {
  public:
    // SpotLight Public Methods
    SpotLight(const Transform &T, const Color3f &emittedLight, Float totalWidth, Float falloffStart);
    virtual Color3f Sample_Li(const Base_Intersection &ref, const Point2f &u, Vector3f *wi, Float *pdf, Intersection *LightSample=nullptr) const;
    Float Falloff(const Vector3f &w) const;
    virtual Float Pdf_Li(const Base_Intersection &, const Vector3f &) const;
    virtual Point3f GetCenter() const;

  private:
    // SpotLight Private Data
    const Point3f pLight;
    const Color3f emittedLight;
    const Float cosTotalWidth, cosFalloffStart;
};
