#include "diffusearealight.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    //TODO
    Color3f color=emittedLight;
    if(!twoSided)
        if(glm::dot(isect.normalGeometric,w)<=0)
            color=Color3f(0.f);
    return color;
}
Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wiw, Float *pdf) const
{
    Intersection I=shape->Sample(ref,xi,pdf);
    if(*pdf==0||glm::length(ref.point-I.point)<0.0001f)
        return Color3f(0.0f);
    *wiw=glm::normalize(I.point-ref.point);
    return L(I,-(*wiw));
}
float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wiW) const
{
    Ray r=ref.SpawnRay(wiW);
    Intersection I;
    if(!(shape->Intersect(r,&I)))
        return 0;
    float cos_value=AbsDot(-wiW,I.normalGeometric);
    if(cos_value==0)
        return 0;
    
    float pdf=glm::length2(I.point-ref.point)/(cos_value*shape->Area());
    return pdf;  
}
