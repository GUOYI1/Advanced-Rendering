#include "orennayarbrdf.h"
#include <warpfunctions.h>

OrenNayarBRDF::OrenNayarBRDF(const Color3f &R, float sig) : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R)
{
    float sigma=glm::radians(sig);
    float sigma_square=sigma*sigma;
    A=1-sigma_square/(2*(sigma_square+0.33f));
    B=0.45f*sigma_square/(sigma_square+0.09f);

}
Color3f OrenNayarBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    float sin_wi=SinTheta(wi);
    float sin_wo=SinTheta(wo);
    float sin_alpha, tan_alpha;
    float maxcos=0.0f;
    if(sin_wi>0.0001&&sin_wo>0.0001)
    {
       float sinphi_wi=SinPhi(wi);
       float cosphi_wi=CosPhi(wi);
       float sinphi_wo=SinPhi(wo);
       float cosphi_wo=CosPhi(wo);
       float cos=cosphi_wi*cosphi_wo+sinphi_wi*sinphi_wo;
       maxcos=glm::max(0.0f,cos);

    }
    if(AbsCosTheta(wi)>AbsCosTheta(wo))
    {
        sin_alpha=sin_wo;
        tan_alpha=sin_wi/AbsCosTheta(wi);
    }
    else
    {
        sin_alpha=sin_wi;
        tan_alpha=sin_wo/AbsCosTheta(wo);
    }
    return R*InvPi*(A+B*maxcos*sin_alpha*tan_alpha);

}
Color3f OrenNayarBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u, Float *pdf, BxDFType *sampledType) const
{
    if(*sampledType==0)
        return Color3f(0.0f);
    *wi=WarpFunctions::squareToHemisphereCosine(u);
    if(wo.z<0.0f)
        wi->z*=-1.0f;
    *pdf=Pdf(wo,*wi);
    return f(wo,*wi);

}
float OrenNayarBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    Point3f sample=wi;
    if(sample.z<0)
        sample.z*=-1;
    return SameHemisphere(wo, wi) ? WarpFunctions::squareToHemisphereCosinePDF(sample):0;
}
