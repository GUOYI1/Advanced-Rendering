#pragma once
#include "material.h"

class TransmissiveMaterial : public Material
{
public:
    TransmissiveMaterial(const Color3f &Kt, float roughness,
                  const std::shared_ptr<QImage> &roughnessMap,
                  float indexOfRefraction,
                  const std::shared_ptr<QImage> &textureMap,
                  const std::shared_ptr<QImage> &normalMap)
        : Kt(Kt), roughness(roughness), roughnessMap(roughnessMap),indexOfRefraction(indexOfRefraction),
          textureMap(textureMap), normalMap(normalMap)
    {}

    void ProduceBSDF(Intersection *isect) const;


private:
    Color3f Kt;                    // The spectral transmission of this material.
                                   // This is just the base color of the material

    float indexOfRefraction;       // The IoR of this glass. We assume the IoR of
                                   // any external medium is that of a vacuum: 1.0

    float roughness;               // The overall roughness of the material. Will be multiplied
                                   // with the roughness map's values.

    std::shared_ptr<QImage> roughnessMap;
    std::shared_ptr<QImage> textureMap; // The color obtained from this (assuming it is non-null) is multiplied with the base material color.
    std::shared_ptr<QImage> normalMap;
};
