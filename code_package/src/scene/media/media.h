#pragma once

#include <globals.h>
#include<raytracing/ray.h>
#include <openGL/drawable.h>
#include <samplers/sampler.h>
inline Float PhaseHG(Float cosTheta, Float g) {
    Float denom = 1 + g * g + 2 * g * cosTheta;
    return Inv4Pi * (1 - g * g) / (denom * std::sqrt(denom));
}

class MediumInteraction;
class Medium {
  public:
    // Medium Interface
    virtual ~Medium() {}
    virtual Color3f Tr(const Ray &ray, Sampler &sampler) const = 0;
    virtual Color3f Sample(const Ray &ray, Sampler &sampler, MediumInteraction *mi) const = 0;
};
struct MediumInterface {
    MediumInterface() : inside(nullptr), outside(nullptr) {}
    // MediumInterface Public Methods
    MediumInterface(const Medium *medium) : inside(medium), outside(medium) {}
    MediumInterface(const Medium *inside, const Medium *outside)
        : inside(inside), outside(outside) {}
    bool IsMediumTransition() const { return inside != outside; }
    const Medium *inside, *outside;
};

class PhaseFunction {
  public:
    // PhaseFunction Interface
    virtual ~PhaseFunction();
    virtual Float p(const Vector3f &wo, const Vector3f &wi) const = 0;
    virtual Float Sample_p(const Vector3f &wo, Vector3f *wi,
                           const Point2f &u) const = 0;
//    virtual std::string ToString() const = 0;
};

class HenyeyGreenstein : public PhaseFunction {
  public:
    // HenyeyGreenstein Public Methods
    HenyeyGreenstein(Float g) : g(g) {}
    Float p(const Vector3f &wo, const Vector3f &wi) const;
    Float Sample_p(const Vector3f &wo, Vector3f *wi,
                   const Point2f &sample) const;
//    std::string ToString() const {
//        return StringPrintf("[ HenyeyGreenstein g: %f ]", g);
//    }

  private:
    const Float g;
};
