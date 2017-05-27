#pragma once

#include "light.h"

class DistanceLight : public Light {
public:
    DistanceLight(const Transform &LightToWorld, const Color3f &emittedLight,
                  const Vector3f &w);
    virtual Color3f Sample_Li(const Base_Intersection &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, Intersection *LightSample=nullptr) const;
    virtual Float Pdf_Li(const Base_Intersection &, const Vector3f &) const;
    virtual Point3f GetCenter() const;

 private:
    const Vector3f wLight;
    const Color3f emittedLight;
};
