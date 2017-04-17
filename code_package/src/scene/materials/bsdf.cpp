#include "bsdf.h"
#include <warpfunctions.h>

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
//TODO: Properly set worldToTangent and tangentToWorld
    : worldToTangent(glm::transpose(glm::mat3(isect.tangent,isect.bitangent,isect.normalGeometric))),
      tangentToWorld(isect.tangent,isect.bitangent,isect.normalGeometric),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{}


void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    //TODO: Update worldToTangent and tangentToWorld based on the normal, tangent, and bitangent
    tangentToWorld=glm::mat3(t,b,n);
    worldToTangent=glm::transpose(tangentToWorld);
}


//
Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    //TODO
    if(numBxDFs==0)
        return Color3f(0.0f);
    Vector3f wo=worldToTangent*woW;
    Vector3f wi=worldToTangent*wiW;
    if(wo.z==0)
        return Color3f(0.0f);
    Color3f result(0.0f);
    //int match_num=0;
    for(int i=0;i<numBxDFs;i++)
    {
        bool reflect = glm::dot(wiW, normal) * glm::dot(woW, normal) > 0;
        if(bxdfs[i]->MatchesFlags(flags)&& \
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION))||\
                 (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
        {
            result+=bxdfs[i]->f(wo,wi);
//            match_num++;
        }
    }
    return result;

}

// Use the input random number _xi_ to select
// one of our BxDFs that matches the _type_ flags.

// After selecting our random BxDF, rewrite the first uniform
// random number contained within _xi_ to another number within
// [0, 1) so that we don't bias the _wi_ sample generated from
// BxDF::Sample_f.

// Convert woW and wiW into tangent space and pass them to
// the chosen BxDF's Sample_f (along with pdf).
// Store the color returned by BxDF::Sample_f and convert
// the _wi_ obtained from this function back into world space.

// Iterate over all BxDFs that we DID NOT select above (so, all
// but the one sampled BxDF) and add their PDFs to the PDF we obtained
// from BxDF::Sample_f, then average them all together.

// Finally, iterate over all BxDFs and sum together the results of their
// f() for the chosen wo and wi, then return that sum.

Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi,
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    //TODO
    //Choose BxDF
    int match_num=BxDFsMatchingFlags(type);
    Color3f f_result(0.f);
    *pdf=0;
    if(sampledType)
        *sampledType=BxDFType(0);
    if(match_num==0)
        return f_result;
    int selected_type=std::min(int(xi[0]*match_num),match_num-1);
    int count=selected_type;
    BxDF* selected_bxdf=nullptr;
    for(int i=0;i<numBxDFs;i++)
    {
        if(bxdfs[i]->MatchesFlags(type)&&(count--)==0)
        {
            selected_bxdf=bxdfs[i];
            break;
        }
    }

    //Sample BxDF
    Point2f new_xi(std::min(xi[0] *match_num-selected_type, OneMinusEpsilon), xi[1]);
    Vector3f wi,wo=worldToTangent*woW;
    if(wo.z==0)
       return Color3f(0.f);

    if(sampledType)
        *sampledType=selected_bxdf->type;
    //f_result=selected_bxdf->(BxDF::Sample_f(wo,&wi,new_xi,pdf,sampledType));
    f_result=selected_bxdf->Sample_f(wo,&wi,new_xi,pdf,sampledType);
    if(*pdf==0)
    {
        if (sampledType)
        {
            *sampledType = BxDFType(0);
        }
        return Color3f(0.f);
    }
    *wiW=tangentToWorld*wi;

    //calculate pdf
    if(!(selected_bxdf->type & BSDF_SPECULAR) && match_num > 1)
    {
        for (int i = 0; i < numBxDFs; i++)
        {
            if (bxdfs[i] != selected_bxdf && bxdfs[i]->MatchesFlags(type))
                *pdf += bxdfs[i]->Pdf(wo, wi);
        }
    }
    if (match_num > 1) *pdf /= match_num;

    //calculate f
    if(!(selected_bxdf->type & BSDF_SPECULAR) && match_num > 1)
    {
       bool reflect = glm::dot(*wiW, normal) * glm::dot(woW, normal) > 0;
       f_result=Color3f(0.0f);
       for(int i=0;i<numBxDFs;i++)
       {
           if(bxdfs[i]->MatchesFlags(type)&&\
                   ((reflect && (bxdfs[i]->type & BSDF_REFLECTION))||\
                    (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
           {
                   f_result+=bxdfs[i]->f(wo, wi);
           }
       }

    }

    return f_result;
}


float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    //TODO
    if(numBxDFs==0)
        return 0;
    Vector3f wo=worldToTangent*woW;
    Vector3f wi=worldToTangent*wiW;
    if(wo.z==0)
        return 0.0f;
    float result=0.0f;
    int match_num=0;
    for(int i=0;i<numBxDFs;i++)
    {
        if(bxdfs[i]->MatchesFlags(flags))
        {
            result+=bxdfs[i]->Pdf(wo,wi);
            match_num++;
        }
    }
    if(match_num!=0)
        result/=match_num;
    return result;

}

Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    //TODO
    if((*sampledType)==0)
        return Color3f(0.0f);
    *wi=WarpFunctions::squareToHemisphereUniform(xi);
    if(wo.z<0)
        wi->z*=-1.0f;
    *pdf=Pdf(wo,*wi);
    return Color3f(0.f);
}

// The PDF for uniform hemisphere sampling
float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? Inv2Pi : 0;
}

BSDF::~BSDF()
{
    for(int i=0;i<numBxDFs;i++)
    {
        delete bxdfs[i];
    }
}
