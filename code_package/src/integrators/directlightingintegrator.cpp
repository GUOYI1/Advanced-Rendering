#include "directlightingintegrator.h"
#include<iostream>
Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth, Color3f beta) const
{
    //TODO
    Color3f color(0.0f);
    Color3f Lr(0,0,0);
    Vector3f woW=-ray.direction;
    Vector3f wiW;
    Intersection intersect;
    if(scene.Intersect(ray,&intersect))
    {
        if(depth<=0||!intersect.objectHit->GetMaterial())
            color=intersect.Le(woW);
        else
        {
            //Randomly select a light
            Point2f xi=sampler->Get2D();
            float pdf=0;
            Color3f f;
            int light_num=scene.lights.size();
            //int select_index=std::min(int(xi[0]*light_num),light_num-1);
            int select_index=std::min(int(sampler->Get1D()*light_num),light_num-1);
            //std::cout<<select_index<<std::endl;
            std::shared_ptr<Light> selected_light=scene.lights[select_index];

            Color3f L_i=selected_light->Sample_Li(intersect,xi,&wiW,&pdf);
            if(selected_light->type==Area)
                pdf/=light_num;
            if(fabs(pdf)==0.0f)
                return intersect.Le(woW);
            wiW=glm::normalize(wiW);

            Ray shadow_Ray=intersect.SpawnRay(wiW);
            Intersection occlusion;
            if(scene.Intersect(shadow_Ray,&occlusion))
            {
                //judge if the shadow ray is occluded
                if(selected_light->type==Area)
                {
                    if(occlusion.objectHit->areaLight!=scene.lights[select_index])
                    //L_i=occlusion.Le(-shadow_Ray.direction);
                    L_i=Color3f(0.0f);
                }
                else if(selected_light->type==Spot||selected_light->type==Point)
                {
                    if(glm::length(occlusion.point-shadow_Ray.origin)<\
                            glm::length(selected_light->GetCenter()-shadow_Ray.origin))
                        L_i=Color3f(0.0f);
                }
                else if(selected_light->type==Distance)
                     L_i=Color3f(0.0f);
            }

            f=intersect.bsdf->f(woW,wiW);
            Lr=f*AbsDot(intersect.normalGeometric,wiW)*L_i/pdf;
            color=intersect.Le(woW)+Lr;
        }

    }
    return color;
//    return Color3f(0.f);
}
