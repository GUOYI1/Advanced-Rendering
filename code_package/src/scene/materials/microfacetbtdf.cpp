#include"microfacetbtdf.h"
Color3f MicrofacetBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO!
    Float cosThetaO=AbsCosTheta(wo),cosThetaI=AbsCosTheta(wi);
    if(cosThetaO==0||cosThetaI==0)
        return Color3f(0.f);
    Float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = wo + wi * eta;
    if (wh.x == 0 && wh.y == 0 && wh.z == 0)
        return Color3f(0.f);
    wh=glm::normalize(wh);
    Float ih=glm::dot(wi,wh);
    Float oh=glm::dot(wo,wh);
    Float denominator=pow(eta*ih+oh,2)*cosThetaO*cosThetaI;
    if(fabs(denominator)<0.00001f)
        return Color3f(0.0f);
    Color3f f=Color3f(1.0f)-fresnel->Evaluate(glm::dot(wi,wh));
    return R*distribution->D(wh)*distribution->G(wo, wi) * f*eta*eta*AbsDot(wi,wh)*AbsDot(wo,wh)/denominator;
}

Color3f MicrofacetBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const
{
    //TODO
    Vector3f wh = distribution->Sample_wh(wo, xi);
    Float eta = CosTheta(wo) > 0 ? (etaA / etaB) : (etaB / etaA);
    if (!Refract(wo, (Normal3f)wh, eta, wi))
        return Color3f(0.0f);
    if(glm::length(*wi)==0)
        return Color3f(0.0f);
    (*wi)=glm::normalize(*wi);
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float MicrofacetBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    if(SameHemisphere(wo,wi))
        return 0.0f;
    Float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = glm::normalize(wo + wi * eta);
    Float etaI=CosTheta(wo) > 0 ? etaB : etaA;
    Float etaO=CosTheta(wo) > 0 ? etaA : etaB;
    Float numerator=etaO*etaO*AbsDot(wo,wh);
    Float v=etaI*glm::dot(wi,wh)+etaO*glm::dot(wo,wh);
    Float denominator=v*v;
    Float dw_dh=numerator/denominator;
    return distribution->Pdf(wo,wh)*dw_dh;
}


