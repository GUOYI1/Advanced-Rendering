#include "fresnel.h"

Color3f FresnelDielectric::Evaluate(float cosThetaI) const
{
    //TODO
    cosThetaI=glm::clamp(cosThetaI,-1.0f,1.0f);
    bool direction_test=cosThetaI>0.0f;
    Float etaI_temp=etaI,etaT_temp=etaT;
    if(!direction_test)
    {
        std::swap(etaI_temp,etaT_temp);
        cosThetaI=std::fabs(cosThetaI);
    }
    //Computing cosThetaT using Snell's Law
    Float sinThetaI=std::sqrt(std::max(0.0f,1-cosThetaI*cosThetaI));
    Float sinThetaT=etaI_temp/etaT_temp*sinThetaI;
    Float cosThetaT=std::sqrt(std::max(0.0f,1-sinThetaT*sinThetaT));

//    //Handle internal reflection
    if(sinThetaT>1.0f)
        return Color3f(1.0f,1.0f,1.0f);
    Float r_parl=((etaT_temp * cosThetaI) - (etaI_temp * cosThetaT))/
            ((etaT_temp * cosThetaI) + (etaI_temp * cosThetaT));
    Float r_perp=((etaI_temp * cosThetaI) - (etaT_temp * cosThetaT))/
            ((etaI_temp * cosThetaI) + (etaT_temp * cosThetaT));
    Float result=(r_parl*r_parl+r_perp*r_perp)/2;
    return Color3f(1.0f)*result;
    //return Color3f(0.0f);

}
