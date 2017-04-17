#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth, Color3f beta) const
{
    //TODO

    Color3f color(0.0f);
    Color3f Lr(0,0,0);
    Vector3f woW=-ray.direction;
    Vector3f wiW;
    Intersection intersect;
    if(scene.Intersect(ray,&intersect))
    {
        color=beta*intersect.Le(woW);
        if(recursionLimit-depth>=3)
        {
            Float q = std::max((Float).05, 1.0f - beta.y);
            if (sampler->Get1D() < q)
                return color;
            beta /= 1.0f - q;
        }
        if(depth<=0||!intersect.objectHit->GetMaterial())
            return color;
        else
        {
            //intersect.ProduceBSDF();
            Point2f xi;
            float pdf=0;
            BxDFType sampletype=BxDFType(0);
            Color3f f;
            //Avoid the wiW perpendicular with intersect.normalGeometric
            xi=sampler->Get2D();
            f=intersect.bsdf->Sample_f(woW,&wiW,xi,&pdf,BSDF_ALL,&sampletype);
            wiW=glm::normalize(wiW);

            Ray tracer=intersect.SpawnRay(wiW);
            if(pdf<=0)
                Lr=Color3f(0.0f);
            else
            {
                beta*=f*AbsDot(intersect.normalGeometric,wiW)/pdf;
                Lr=Li(tracer,scene,sampler,depth-1,beta);
            }
        }
         color+=Lr;
    }
    return color;
}
