#pragma once
#include "bsdf.h"

class OrenNayarBRDF : public BxDF
{
public:
    OrenNayarBRDF(const Color3f &R,float sig);

    Color3f f(const Vector3f &wo, const Vector3f &wi) const;

    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &u, Float *pdf,
                              BxDFType *sampledType = nullptr) const;
    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;


private:
    const Color3f R; // The energy scattering coefficient of this BRDF (i.e. its color)
    float A;
    float B;
};
