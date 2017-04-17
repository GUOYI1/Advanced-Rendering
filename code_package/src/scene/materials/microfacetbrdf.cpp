#include "microfacetbrdf.h"

Color3f MicrofacetBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO!
    Float cosThetaO=AbsCosTheta(wo),cosThetaI=AbsCosTheta(wi);
    if(cosThetaO==0||cosThetaI==0)
        return Color3f(0.f);
    Vector3f wh=wi+wo;
    if (wh.x == 0 && wh.y == 0 && wh.z == 0)
        return Color3f(0.f);
    wh=glm::normalize(wh);
    Color3f f=fresnel->Evaluate(glm::dot(wi,wh));
    return R*distribution->D(wh)*distribution->G(wo, wi) * f /(4*cosThetaI*cosThetaO);
}

Color3f MicrofacetBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const
{
    //TODO
    Vector3f wh=distribution->Sample_wh(wo,xi);
    *wi=glm::reflect(-wo,wh);
    if(!SameHemisphere(wo,*wi))
        return Color3f(0.0f);
    *pdf = distribution->Pdf(wo, wh)/(4*glm::dot(wo,wh));
    return f(wo,*wi);
}

float MicrofacetBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    if(!SameHemisphere(wo,wi))
        return 0.0f;
    Vector3f wh=glm::normalize(wo+wi);
    return distribution->Pdf(wo,wh)/(4*glm::dot(wo,wh));
}
