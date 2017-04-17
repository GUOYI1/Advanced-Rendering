#include "lambertbrdf.h"
#include <warpfunctions.h>
#include<iostream>

Color3f LambertBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO   
    return R/Pi;
}

Color3f LambertBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    //Todo
    if(*sampledType==0)
        return Color3f(0.0f);
    *wi=WarpFunctions::squareToHemisphereCosine(u);
    //*wi=WarpFunctions::squareToHemisphereUniform(u);
    if(wo.z<0.0f)
        wi->z*=-1.0f;
    *pdf=Pdf(wo,*wi);
    return f(wo,*wi);

}

float LambertBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO    
    Point3f sample=wi;
    if(sample.z<0)
        sample.z*=-1;
    //return SameHemisphere(wo, wi) ? WarpFunctions::squareToHemisphereUniformPDF(sample):0;
    return SameHemisphere(wo, wi) ? WarpFunctions::squareToHemisphereCosinePDF(sample):0;

}
