#include "fulllightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth, Color3f beta) const
{
/////////////////////////////**hw6**//////////////////////////////
//    //TODO
//    Color3f color(0.0f);
//    Color3f Ld(0,0,0);
//    Color3f Le(0,0,0);
//    Vector3f woW=-ray.direction;
//    Vector3f wiW;
//    Intersection intersect;
//    if(scene.Intersect(ray,&intersect))
//    {
//        color=beta*intersect.Le(woW);
//        if(recursionLimit-depth>=3)
//        {
//            Float temp=std::max(beta.x,beta.y);
//            Float max_componenet=std::max(temp,beta.z);
//            Float q = std::max((Float).05, 1.0f - max_componenet);
//            if (sampler->Get1D() < q)
//                return color;
//            beta /= 1.0f - q;
//        }

//        if(depth<=0||!intersect.objectHit->GetMaterial())
//            return color;
//        else
//        {
//            //Light source Sample
//            //Randomly select a light
//            Point2f xi=sampler->Get2D();
//            float lightPdf=0,scatteringPdf=0;
//            float lightweight=0, scatterWeight=0;
//            Color3f f_light,f_scattering;
//            int light_num=scene.lights.size();
//            int select_index=std::min(int(sampler->Get1D()*light_num),light_num-1);
//            std::shared_ptr<Light> selected_light=scene.lights[select_index];

//            Color3f L_i=selected_light->Sample_Li(intersect,xi,&wiW,&lightPdf);

//            lightPdf/=light_num;
//            if(fabs(lightPdf)==0.0f)
//                L_i=Color3f(0.0f);
//            wiW=glm::normalize(wiW);
//            Ray Trace_Ray=intersect.SpawnRay(wiW);
//            Intersection occlusion;
//            if(scene.Intersect(Trace_Ray,&occlusion))
//            {
//                //judge if the Trace ray is occluded
//                if(occlusion.objectHit->areaLight!=scene.lights[select_index])
//                    L_i=Color3f(0.0f);
//            }
//            f_light=intersect.bsdf->f(woW,wiW);
//            scatteringPdf=intersect.bsdf->Pdf(woW,wiW);
//            lightweight=PowerHeuristic(1,lightPdf,1,scatteringPdf);
//            //lightweight=BalanceHeuristic(1,lightPdf,1,scatteringPdf);
//            Ld+=f_light*AbsDot(intersect.normalGeometric,wiW)*L_i*lightweight/lightPdf;

////////////////////////////////////////////////////////////////////////////
//            //Remap xi
//            xi=sampler->Get2D();
//            BxDFType sampletype=BxDFType(0);
//            f_scattering=intersect.bsdf->Sample_f(woW,&wiW,xi,&scatteringPdf,BSDF_ALL,&sampletype);
//            wiW=glm::normalize(wiW);
//            lightPdf = selected_light->Pdf_Li(intersect,wiW)/light_num;
//            //uncomment this and comment the code behind if you want to test the code with 2 sample directions
////            if(scatteringPdf>0)
////            {
////                Trace_Ray=intersect.SpawnRay(wiW);

////                scatterWeight=PowerHeuristic(1,scatteringPdf,1,lightPdf);
////                Ld+=f_scattering*AbsDot(intersect.normalGeometric,wiW)*Li(Trace_Ray,scene,sampler,depth-1)\
////                        *scatterWeight/scatteringPdf;
////            }

////           Ensure pdf of light is not zero, since we only want to sample light for Ld
//            Intersection occlusion2;
//            if(lightPdf!=0&&scatteringPdf!=0)
//            {
//                Trace_Ray=intersect.SpawnRay(wiW);
//                if(scene.Intersect(Trace_Ray,&occlusion2))
//                {
//                    //judge if the shadow ray is occluded
//                    if(occlusion2.objectHit->areaLight==selected_light)
//                    {
//                        scatterWeight=PowerHeuristic(1,scatteringPdf,1,lightPdf);
////                        scatterWeight=BalanceHeuristic(1,scatteringPdf,1,lightPdf);
//                        L_i=occlusion2.Le(-wiW);
//                        Ld+=f_scattering*AbsDot(intersect.normalGeometric,wiW)*L_i*scatterWeight/scatteringPdf;
//                    }
//                }
//            }
//            Ld*=beta;
////////////////////////////////////////////////////////////////////////////////
//            //Indirect Light sampel
//            xi=sampler->Get2D();
//            float pdf=0;
//            Color3f f;
//            f=intersect.bsdf->Sample_f(woW,&wiW,xi,&pdf,BSDF_ALL,&sampletype);
//            wiW=glm::normalize(wiW);
//            Trace_Ray=intersect.SpawnRay(wiW);
//            Intersection BISisect;
//            if(fabs(pdf)<=0)
//                Le=Color3f(0.0f);
//            else if(scene.Intersect(Trace_Ray,&BISisect))
//            {
//                if(BISisect.objectHit->areaLight)
//                    Le=Color3f(0.0f);
//                else
//                {
//                    beta*=f*AbsDot(intersect.normalGeometric,wiW)/pdf;
//                    //Here I multiply the beta inside the Function Li, so I should multiply
//                    //the f*AbsDot/pdf again outside the Li
//                    Le=Li(Trace_Ray,scene,sampler,depth-1,beta);
//                }
//            }
//            else
//                Le=Color3f(0.0f);

//        }
//        color+=(Ld+Le);
//    }
//    return color;
/////////////////////////////**hw7**/////////////////////////////
     Color3f L(0.0f);
     Color3f beta_loop(1.0f);
     Ray r=ray;
     Vector3f woW;
     Vector3f wiW;
     Point2f xi;
     BxDFType sampletype;
     bool specular_check=false;
     while(depth>0)
     {
         sampletype=BxDFType(0);
         woW=-r.direction;
         Intersection intersect;
         bool intersection_test=scene.Intersect(r,&intersect);
         if(!intersection_test)
             break;
         if(depth==recursionLimit||specular_check)
             L+=beta_loop*intersect.Le(woW);
         if(intersect.objectHit->areaLight)
             break;
         //MIS
         Color3f Ld(0.0f);
         xi=sampler->Get2D();
         float lightPdf=0,scatteringPdf=0;
         float lightweight=0, scatterWeight=0;
         Color3f f_light,f_scattering;
         int light_num=scene.lights.size();
         int select_index=std::min(int(sampler->Get1D()*light_num),light_num-1);
         std::shared_ptr<Light> selected_light=scene.lights[select_index];

         Color3f L_i=selected_light->Sample_Li(intersect,xi,&wiW,&lightPdf);

         //lightPdf/=light_num;
         if(fabs(lightPdf)==0.0f)
             L_i=Color3f(0.0f);
         wiW=glm::normalize(wiW);
         f_light=intersect.bsdf->f(woW,wiW);
         if(glm::length(f_light-Color3f(0,0,0))>FLT_EPSILON)
         {
             r=intersect.SpawnRay(wiW);
             Intersection occlusion;
             if(scene.Intersect(r,&occlusion))
             {
                 //judge if the Trace ray is occluded
                 if(selected_light->type==Area)
                 {
                     if(occlusion.objectHit->areaLight!=scene.lights[select_index])
                         //L_i=occlusion.Le(-shadow_Ray.direction);
                         L_i=Color3f(0.0f);
                 }
                 else if(selected_light->type==Spot||selected_light->type==Point)
                 {
                     if(glm::length(occlusion.point-r.origin)<\
                             glm::length(selected_light->GetCenter()-r.origin))
                         L_i=Color3f(0.0f);
                 }
                 else if(selected_light->type==Distance)
                     L_i=Color3f(0.0f);
             }

             scatteringPdf=intersect.bsdf->Pdf(woW,wiW);
             lightweight=PowerHeuristic(1,lightPdf,1,scatteringPdf);
             //lightweight=BalanceHeuristic(1,lightPdf,1,scatteringPdf);
             Ld+=f_light*AbsDot(intersect.bsdf->normal,wiW)*L_i*lightweight/lightPdf;
         }
         //////////////////////////////////////////////////////////////////////////
         //Remap xi
         xi=sampler->Get2D();
         f_scattering=intersect.bsdf->Sample_f(woW,&wiW,xi,&scatteringPdf,BSDF_ALL,&sampletype);
         specular_check=(sampletype&BSDF_SPECULAR) != 0;
         wiW=glm::normalize(wiW);
         //lightPdf = selected_light->Pdf_Li(intersect,wiW)/light_num;
         if(!specular_check)
         {
             lightPdf = selected_light->Pdf_Li(intersect,wiW);
             Intersection occlusion2;
             if(lightPdf!=0&&scatteringPdf!=0)
             {
                 r=intersect.SpawnRay(wiW);
                 if(scene.Intersect(r,&occlusion2))
                 {
                     //judge if the shadow ray is occluded
                     if(occlusion2.objectHit->areaLight==selected_light)
                     {
                         scatterWeight=PowerHeuristic(1,scatteringPdf,1,lightPdf);
                         //scatterWeight=BalanceHeuristic(1,scatteringPdf,1,lightPdf);
                         L_i=occlusion2.Le(-wiW);
                         Ld+=f_scattering*AbsDot(intersect.bsdf->normal,wiW)*L_i*scatterWeight/scatteringPdf;
                     }
                 }
             }             
         }
         Ld*=beta_loop;
         if(selected_light->type==Area)
             Ld*=light_num;
         L+=Ld;
         //BIS
         xi=sampler->Get2D();
         float pdf=0;
         Color3f f;
         f=intersect.bsdf->Sample_f(woW,&wiW,xi,&pdf,BSDF_ALL,&sampletype);
         wiW=glm::normalize(wiW);
         if(fabs(pdf)==0||glm::length(f-Color3f(0.0f))==0.0f)
             break;
         beta_loop*=f*AbsDot(intersect.bsdf->normal,wiW)/pdf;
         specular_check=(sampletype&BSDF_SPECULAR) != 0;
         r=intersect.SpawnRay(wiW);
         if(recursionLimit-depth>=3)
         {
             Float temp=std::max(beta.x,beta.y);
             Float max_componenet=std::max(temp,beta.z);
             Float q = std::max((Float).05, 1.0f - max_componenet);
             if (sampler->Get1D() < q)
                 break;
             beta /= 1.0f - q;
         }
         depth--;


     }
     return L;

}



