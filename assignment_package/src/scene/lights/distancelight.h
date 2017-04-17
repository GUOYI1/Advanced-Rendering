#pragma once

#include "light.h"

class DistanceLight : public Light {
public:
    DistanceLight(const Transform &LightToWorld, const Color3f &emittedLight,
                  const Vector3f &w);
    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf) const;
    virtual Float Pdf_Li(const Intersection &, const Vector3f &) const;
    virtual Point3f GetCenter();

 private:
    const Vector3f wLight;
    const Color3f emittedLight;
};
