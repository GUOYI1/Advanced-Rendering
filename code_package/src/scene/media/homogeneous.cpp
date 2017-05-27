#pragma once
#include"homogeneous.h"
#include <samplers/sampler.h>
#include<raytracing/intersection.h>
static int nSamples=3;
Color3f HomogeneousMedium::Tr(const Ray &ray, Sampler &sampler) const {
    return glm::exp(-sigma_t * std::min(ray.tMax * glm::length(ray.direction), MaxFloat));
}

Color3f HomogeneousMedium::Sample(const Ray &ray, Sampler &sampler,
                                   MediumInteraction *mi) const {
    // Sample a channel and distance along the ray
    int channel = std::min((int)(sampler.Get1D() * nSamples),nSamples - 1);
    Float dist = -std::log(1 - sampler.Get1D()) / sigma_t[channel];
    Float t = std::min(dist * glm::length(ray.direction), ray.tMax);
    bool sampledMedium = t < ray.tMax;
    std::shared_ptr<PhaseFunction>phase=std::make_shared<HenyeyGreenstein>(g);
    if (sampledMedium)
    {
        *mi = MediumInteraction(ray(t), ray.tMax, this,phase);
    }

    // Compute the transmittance and sampling density
    Color3f Tr = glm::exp(-sigma_t * std::min(t, MaxFloat) *  glm::length(ray.direction));

    // Return weighting factor for scattering from homogeneous medium
    Color3f density = sampledMedium ? (sigma_t * Tr) : Tr;
    Float pdf = 0;
    for (int i = 0; i <nSamples; ++i) pdf += density[i];
    pdf *= 1 / (Float)nSamples;
    if (pdf == 0) {
            pdf = 1;
    }
    return sampledMedium ? (Tr * sigma_s / pdf) : (Tr / pdf);
}

