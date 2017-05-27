#include "integrator.h"


void Integrator::run()
{
    Render();
}

void Integrator::Render()
{
    // Compute the bounds of our sample, clamping to screen's max bounds if necessary
    // Instantiate a FilmTile to store this thread's pixel colors
    std::vector<Point2i> tilePixels = bounds.GetPoints();
    // For every pixel in the FilmTile:
    for(Point2i pixel : tilePixels)
    {
        //Uncomment this to debug a particular pixel within this tile
//        if(pixel.x != 200 || pixel.y != 360)
//        {
//            continue;
//        }
        Color3f pixelColor(0.f);
        // Ask our sampler for a collection of stratified samples, then raycast through each sample
        std::vector<Point2f> pixelSamples = sampler->GenerateStratifiedSamples();
        for(Point2f sample : pixelSamples)
        {
            sample = sample + Point2f(pixel); // _sample_ is [0, 1), but it needs to be translated to the pixel's origin.
            // Generate a ray from this pixel sample
            Ray ray = camera->Raycast(sample);
            // Get the L (energy) for the ray by calling Li(ray, scene, tileSampler, arena)
            // Li is implemented by Integrator subclasses, like DirectLightingIntegrator
            Color3f L = Li(ray, *scene, sampler, recursionLimit,Color3f(1.0f));
            // Accumulate color in the pixel
            pixelColor += L;
        }
        // Average all samples' energies
        pixelColor /= pixelSamples.size();
        film->SetPixelColor(pixel, glm::clamp(pixelColor, 0.f, 1.f));
    }
    //We're done here! All pixels have been given an averaged color.
}


void Integrator::ClampBounds()
{
    Point2i max = bounds.Max();
    max = Point2i(std::min(max.x, film->bounds.Max().x), std::min(max.y, film->bounds.Max().y));
    bounds = Bounds2i(bounds.Min(), max);
}
Color3f EstimateDirect(const Base_Intersection &it, const Vector3f &wo, const Point2f &uScattering,
                        const Light &light, const Point2f &uLight,
                        const Scene &scene, Sampler &sampler,
                        bool handleMedia, bool specular) {
    BxDFType bsdfFlags = specular ? BSDF_ALL : BxDFType(BSDF_ALL & ~BSDF_SPECULAR);
    Color3f Ld(0.f);
    // Sample light source with multiple importance sampling
    Vector3f wi;
    Float lightPdf = 0, scatteringPdf = 0;
    //VisibilityTester visibility;
    Intersection LightSample;
    Color3f Li = light.Sample_Li(it, uLight, &wi, &lightPdf, &LightSample);

    if (lightPdf > 0 && Li!=Color3f(0.0f)) {
        // Compute BSDF or phase function's value for light sample
        Color3f f;
        if (it.IsSurfaceInteraction()) {
            // Evaluate BSDF for light sampling strategy
            const Intersection &isect = (const Intersection &)it;
            f = isect.bsdf->f(wo, wi, bsdfFlags)*AbsDot(wi, isect.bsdf->normal);
            scatteringPdf = isect.bsdf->Pdf(wo, wi, bsdfFlags);
        } else {
            // Evaluate phase function for light sampling strategy
            const MediumInteraction &mi = (const MediumInteraction &)it;
            Float p = mi.phase->p(wo, wi);
            f = Color3f(p);
            scatteringPdf = p;
        }
        if (f!=Color3f(0.0f)) {
            // Compute effect of visibility for light source sample
            if (handleMedia) {
                //Li *= visibility.Tr(scene, sampler);
                Ray ray=it.SpawnRayTo(LightSample);
                if(light.type==Distance||light.type==Infinite)
                    ray=it.SpawnRay(wi);

                Color3f Tr(1.f);
                while (true)
                {
                    Intersection isect;
                    bool hitSurface = scene.Intersect(ray, &isect);
                    // Handle opaque surface along ray's path
                    if (hitSurface && (isect.objectHit->GetMaterial() != nullptr||\
                                       isect.objectHit->GetAreaLight()!=&light))
                    {
                        Tr=Color3f(0.0f);
                        break;
                    }

                    // Update transmittance for current ray segment
                    if (ray.medium)
                    {
                        Tr *= ray.medium->Tr(ray, sampler);
                    }
                    // Generate next ray segment or return final transmittance
                    if (!hitSurface) break;

                    if(light.type==Distance||light.type==Infinite)
                        ray=isect.SpawnRay(wi);
                    else
                        ray=isect.SpawnRayTo(LightSample);
                }
                Li *=Tr;
            } else {
                Ray shadow_Ray=it.SpawnRay(wi);
                Intersection occlusion;
                if(scene.Intersect(shadow_Ray,&occlusion))
                {
                    //judge if the shadow ray is occluded
                    if(light.type==Area)
                    {
                        if(occlusion.objectHit->GetAreaLight()!=&light)
                        //L_i=occlusion.Le(-shadow_Ray.direction);
                            Li=Color3f(0.0f);
                    }
                    else if(light.type==Spot||light.type==Point)
                    {
                        if(glm::length(occlusion.point-shadow_Ray.origin)<\
                                glm::length(light.GetCenter()-shadow_Ray.origin))
                            Li=Color3f(0.0f);
                    }
                    else if(light.type==Distance)
                         Li=Color3f(0.0f);
                }
            }

            // Add light's contribution to reflected radiance
            if (Li!=Color3f(0.0f)) {
                if (IsDeltaLight(light.type))
                    Ld += f * Li / lightPdf;
                else {
                    Float weight = PowerHeuristic(1, lightPdf, 1, scatteringPdf);
                    Ld += f * Li * weight / lightPdf;
                }
            }
        }
    }

    // Sample BSDF with multiple importance sampling
    if (!IsDeltaLight(light.type)) {
        Color3f f;
        bool sampledSpecular = false;
        if (it.IsSurfaceInteraction()) {
            // Sample scattered direction for surface Intersections
            BxDFType sampledType;
            const Intersection &isect = (const Intersection &)it;
            f = isect.bsdf->Sample_f(wo, &wi, uScattering, &scatteringPdf,
                                     bsdfFlags, &sampledType);
            f *= AbsDot(wi, isect.bsdf->normal);
            sampledSpecular = (sampledType & BSDF_SPECULAR) != 0;
        } else {
            // Sample scattered direction for medium Intersections
            const MediumInteraction &mi = (const MediumInteraction &)it;
            Float p = mi.phase->Sample_p(wo, &wi, uScattering);
            f = Color3f(p);
            scatteringPdf = p;
        }
        if (f!=Color3f(0.0f) && scatteringPdf > 0.0f) {
            // Account for light contributions along sampled direction _wi_
            Float weight = 1;
            if (!sampledSpecular) {
                lightPdf = light.Pdf_Li(it, wi);
                if (lightPdf == 0) return Ld;
                weight = PowerHeuristic(1, scatteringPdf, 1, lightPdf);
            }

            // Find intersection and compute transmittance
            Intersection lightIsect;
            Ray ray = it.SpawnRay(wi);
            Color3f Tr(1.f);
            bool foundSurfaceIntersection =
                handleMedia ? scene.IntersectTr(ray, sampler, &lightIsect, &Tr)
                            : scene.Intersect(ray, &lightIsect);

            // Add light contribution from material sampling
            Color3f Li(0.f);
            if (foundSurfaceIntersection) {
                if (lightIsect.objectHit->GetAreaLight() == &light)
                    Li = lightIsect.Le(-wi);
            } else
                Li = light.Le(ray);
            if (Li!=Color3f(0.0f)) Ld += f * Li * Tr * weight / scatteringPdf;
        }
    }
    return Ld;
}
Color3f UniformSampleOneLight(const Base_Intersection &it,const Vector3f &wo,const Scene &scene,
                               Sampler &sampler, bool handleMedia)
{
    // Randomly choose a single light to sample, _light_
    int nLights = int(scene.lights.size());
    if (nLights == 0) return Color3f(0.f);
    int lightNum;
    Float lightPdf;
    lightNum = std::min((int)(sampler.Get1D() * nLights), nLights - 1);
    lightPdf = Float(1) / nLights;

    const std::shared_ptr<Light> &light = scene.lights[lightNum];
    Point2f uLight = sampler.Get2D();
    Point2f uScattering = sampler.Get2D();
    return EstimateDirect(it,wo,uScattering, *light, uLight,
                          scene, sampler, handleMedia) / lightPdf;
}
