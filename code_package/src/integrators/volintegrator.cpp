#include"volintegrator.h"
Color3f VolIntegrator::Li(const Ray &r, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth, Color3f beta) const
{
     Color3f L(0.f), beta_loop(1.f);
     Ray ray(r);
     bool specularBounce = false;
     int bounces;
     // Added after book publication: etaScale tracks the accumulated effect
     // of radiance scaling due to rays passing through refractive
     // boundaries (see the derivation on p. 527 of the third edition). We
     // track this value in order to remove it from beta_loop when we apply
     // Russian roulette; this is worthwhile, since it lets us sometimes
     // avoid terminating refracted rays that are about to be refracted back
     // out of a medium and thus have their beta_loop value increased.
     Float etaScale = 1;

     for (bounces = 0;; ++bounces) {
         // Intersect _ray_ with scene and store intersection in _isect_
         Intersection isect;
         bool foundIntersection = scene.Intersect(ray, &isect);
         MediumInteraction mi;
         if (ray.medium)
         {
             beta_loop *= ray.medium->Sample(ray, *sampler,&mi);
         }
         if (beta_loop==Color3f(0.0f))
         {
             break;
         }
         // Handle an interaction with a medium or a surface
         if (mi.IsValid())
         {
             // Terminate path if ray escaped or _maxDepth_ was reached
             if (bounces >= recursionLimit) break;
             // Handle scattering at point in medium for volumetric path tracer

             Vector3f wo = -ray.direction, wi;
             L += beta_loop * UniformSampleOneLight(mi, wo,scene,*sampler, true);


             mi.phase->Sample_p(wo, &wi, sampler->Get2D());
             ray = mi.SpawnRay(wi);
         }
         else
         {

             // Handle scattering at point on surface for volumetric path tracer

             // Possibly add emitted light at intersection
             if (bounces == 0 || specularBounce) {
                 // Add emitted light at path vertex or from the environment
                 if (foundIntersection)
                     L += beta_loop * isect.Le(-ray.direction);
                 else
                     for (const auto &light : scene.infiniteLights)
                         L += beta_loop * light->Le(ray);
             }

             // Terminate path if ray escaped or _maxDepth_ was reached
             if (!foundIntersection || bounces >= recursionLimit) break;

             // Compute scattering functions and skip over medium boundaries
             if (!isect.bsdf) {
                 ray = isect.SpawnRay(ray.direction);
                 bounces--;
                 continue;
             }

             // Sample illumination from lights to find attenuated path
             // contribution
             Vector3f wo = -ray.direction, wi;
             L += beta_loop * UniformSampleOneLight(isect, wo,scene,*sampler,
                                               true);

             // Sample BSDF to get new path direction

             Float pdf;
             BxDFType flags;
             Color3f f = isect.bsdf->Sample_f(wo, &wi, sampler->Get2D(), &pdf,
                                               BSDF_ALL, &flags);
             if (f==Color3f(0.f) || pdf == 0.f) break;
             beta_loop *= f * AbsDot(wi, isect.bsdf->normal) / pdf;

             specularBounce = (flags & BSDF_SPECULAR) != 0;
             if ((flags & BSDF_SPECULAR) && (flags & BSDF_TRANSMISSION)) {
                 Float eta = isect.bsdf->eta;
                 // Update the term that tracks radiance scaling for refraction
                 // depending on whether the ray is entering or leaving the
                 // medium.
                 etaScale *= (glm::dot(wo, isect.normalGeometric) > 0) ? (eta * eta) : 1 / (eta * eta);
             }
             ray = isect.SpawnRay(wi);

         }

         // Possibly terminate the path with Russian roulette
         // Factor out radiance scaling due to refraction in rrbeta_loop.
         Color3f rrbeta_loop = beta_loop * etaScale;
         Float temp=std::max(rrbeta_loop.x,rrbeta_loop.y);
         Float max_componenet=std::max(temp,rrbeta_loop.z);
         if (max_componenet < rrThreshold && bounces > 3) {
             Float q = std::max((Float).05, 1 - max_componenet);
             if (sampler->Get1D() < q) break;
             beta_loop /= 1 - q;
         }
     }
     return L;
}
