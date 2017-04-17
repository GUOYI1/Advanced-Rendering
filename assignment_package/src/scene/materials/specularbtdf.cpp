#include "specularbTdf.h"

Color3f SpecularBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    //TODO!
    bool direction_test=CosTheta(wo)>0;
    Float etaI=direction_test? etaA:etaB;
    Float etaT=direction_test? etaB:etaA;
    //handle the internal reflection
    if(!Refract(wo,Faceforward(Normal3f(0,0,1),wo),etaI/etaT,wi))
        return Color3f(0.f);
    *pdf=1;
    Color3f f_T=T*(Color3f(1.0f)-fresnel->Evaluate(CosTheta(*wi)));
    return Color3f(f_T)/AbsCosTheta(*wi);
}
