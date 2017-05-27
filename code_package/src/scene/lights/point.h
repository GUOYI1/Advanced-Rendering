#pragma once

#include "light.h"

class PointLight : public Light {
public:
    PointLight(const Transform &T, const Color3f &emittedLight);
    virtual Color3f Sample_Li(const Base_Intersection &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, Intersection *LightSample=nullptr) const;
    virtual Float Pdf_Li(const Base_Intersection &, const Vector3f &) const;
    virtual Point3f GetCenter() const;

  private:
    const Point3f pLight;
    const Color3f emittedLight;
};
