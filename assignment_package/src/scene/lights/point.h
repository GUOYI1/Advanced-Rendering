#pragma once

#include "light.h"

class PointLight : public Light {
public:
    PointLight(const Transform &T, const Color3f &emittedLight);
    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf) const;
    virtual Float Pdf_Li(const Intersection &, const Vector3f &) const;
    virtual Point3f GetCenter();

  private:
    const Point3f pLight;
    const Color3f emittedLight;
};
