#pragma once
#include "bsdf.h"
#include "fresnel.h"
#include "microfacet.h"

class MicrofacetBTDF : public BxDF
{
public:
    MicrofacetBTDF(const Color3f &R, MicrofacetDistribution* distribution, Float etaA,Float etaB,Fresnel*fresnel)
        : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)), R(R), distribution(distribution), etaA(etaA),\
          etaB(etaB),fresnel(fresnel) {}

    virtual ~MicrofacetBTDF(){delete distribution;}

    Color3f f(const Vector3f &wo, const Vector3f &wi) const;

    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &xi, Float *pdf,
                              BxDFType *sampledType = nullptr) const;
    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;


  private:
    const Color3f R; // The energy scattering coefficient of this BRDF (i.e. its color)
    const MicrofacetDistribution* distribution;
    const Fresnel* fresnel; // A class that will determine the reflectivity coefficient at this point
    const Float etaA, etaB;
};

