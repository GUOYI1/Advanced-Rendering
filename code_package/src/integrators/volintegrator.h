#pragma once
#include "integrator.h"
class VolIntegrator : public Integrator
{
public:
    VolIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit,Float rrThreshold)
        : Integrator(bounds, s, sampler, recursionLimit),rrThreshold(rrThreshold)
    {}

    // Evaluate the energy transmitted along the ray back to
    // its origin, which can only be the camera in a direct lighting
    // integrator (only rays from light sources are considered)
    virtual Color3f Li(const Ray &r, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth,Color3f beta) const;
private:
    const Float rrThreshold;
};

