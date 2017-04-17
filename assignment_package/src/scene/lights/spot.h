#pragma once

#include "light.h"

class SpotLight : public Light {
  public:
    // SpotLight Public Methods
    SpotLight(const Transform &T, const Color3f &emittedLight, Float totalWidth, Float falloffStart);
    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &u, Vector3f *wi,Float *pdf) const;
    Float Falloff(const Vector3f &w) const;
    virtual Float Pdf_Li(const Intersection &, const Vector3f &) const;
    virtual Point3f GetCenter();

  private:
    // SpotLight Private Data
    const Point3f pLight;
    const Color3f emittedLight;
    const Float cosTotalWidth, cosFalloffStart;
};
