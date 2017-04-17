#include "microfacet.h"

MicrofacetDistribution::~MicrofacetDistribution()
{}

float MicrofacetDistribution::Pdf(const Vector3f &wo, const Vector3f &wh) const
{
    return D(wh) * AbsCosTheta(wh);
}

float TrowbridgeReitzDistribution::D(const Vector3f &wh) const
{
    float tan2Theta = Tan2Theta(wh);
    if (std::isinf(tan2Theta)) return 0.f;

    const float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);

    float e =
            (Cos2Phi(wh) / (alphax * alphax) + Sin2Phi(wh) / (alphay * alphay)) *
            tan2Theta;
    return 1 / (Pi * alphax * alphay * cos4Theta * (1 + e) * (1 + e));
}

float TrowbridgeReitzDistribution::Lambda(const Vector3f &w) const
{
    float absTanTheta = std::abs(TanTheta(w));
    if (std::isinf(absTanTheta)) return 0.;

    // Compute alpha for direction w
    float alpha =
            std::sqrt(Cos2Phi(w) * alphax * alphax + Sin2Phi(w) * alphay * alphay);
    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1 + std::sqrt(1.f + alpha2Tan2Theta)) / 2;
}

Vector3f TrowbridgeReitzDistribution::Sample_wh(const Vector3f &wo, const Point2f &xi) const
{
    Vector3f wh;
    float cosTheta = 0, phi = (2 * Pi) * xi[1];
    if (alphax == alphay) {
        float tanTheta2 = alphax * alphax * xi[0] / (1.0f - xi[0]);
        cosTheta = 1 / std::sqrt(1 + tanTheta2);
    } else {
        phi =
                std::atan(alphay / alphax * std::tan(2 * Pi * xi[1] + .5f * Pi));
        if (xi[1] > .5f) phi += Pi;
        float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
        const float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
        const float alpha2 =
                1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
        float tanTheta2 = alpha2 * xi[0] / (1 - xi[0]);
        cosTheta = 1 / std::sqrt(1 + tanTheta2);
    }
    float sinTheta =
            std::sqrt(std::max((float)0., (float)1. - cosTheta * cosTheta));

    wh = Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi),
                  cosTheta);
    if (!SameHemisphere(wo, wh)) wh = -wh;

    return wh;
}
float BeckmannDistribution::D(const Vector3f &wh) const
{
    Float tan2Theta = Tan2Theta(wh);
    if (std::isinf(tan2Theta)) return 0;
    Float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
    return std::exp(-tan2Theta * (Cos2Phi(wh) / (alphax * alphax) + Sin2Phi(wh) / (alphay * alphay))) /
    (Pi * alphax * alphay * cos4Theta);
}
float BeckmannDistribution::Lambda(const Vector3f &w) const
{
    Float absTanTheta = std::abs(TanTheta(w));
    if (std::isinf(absTanTheta)) return 0.;
    Float alpha = std::sqrt(Cos2Phi(w) * alphax * alphax + Sin2Phi(w) * alphay * alphay);
    Float a = 1 / (alpha * absTanTheta);
    if (a >= 1.6f)
        return 0;
    return (1 - 1.259f * a + 0.396f * a * a) /(3.535f * a + 2.181f * a * a);

}
Vector3f BeckmannDistribution::Sample_wh(const Vector3f &wo, const Point2f &xi) const
{
    Float tan2Theta, phi;
    if (alphax == alphay)
    {
        Float logSample = std::log(xi[0]);
        if (std::isinf(logSample)) logSample = 0;
        tan2Theta = -alphax * alphax * logSample;
        phi = xi[1] * 2 * Pi;
    }
    else
    {
        Float logSample = std::log(xi[0]);
        phi = std::atan(alphay / alphax * std::tan(2 * Pi * xi[1] + .5f * Pi));
        if (xi[1] > .5f) phi += Pi;
        float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
        const float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
        const float alpha2 =1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
        tan2Theta = -alpha2*logSample;
    }

    Float cosTheta = 1 / std::sqrt(1 + tan2Theta);
    Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
    Vector3f wh = SphericalDirection(sinTheta, cosTheta, phi);
    if (!SameHemisphere(wo, wh)) wh = -wh;
    return wh;

}
BlinnDistribution::BlinnDistribution(float e)
{
    if (e > 10000.f || std::isnan(e))
        e = 10000.f;
    exponent = e;
}

float BlinnDistribution::D(const Vector3f &wh) const
{
    float costhetah = AbsCosTheta(wh);
    return (exponent+2) * Inv2Pi * powf(costhetah, exponent);
}

float BlinnDistribution::Lambda(const Vector3f &w) const
{
    float absTanTheta = std::abs(TanTheta(w));
    if (std::isinf(absTanTheta)) return 0.;
    // Compute alpha for direction w
    float alpha =
            std::sqrt(Cos2Phi(w)+ Sin2Phi(w));
    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1 + std::sqrt(1.f + alpha2Tan2Theta)) / 2;
}

Vector3f BlinnDistribution::Sample_wh(const Vector3f &wo, const Point2f &xi) const
{
    Vector3f wh;
    float cosTheta = 0, phi = (2 * Pi) * xi[1];
    cosTheta = pow(xi[0],1/(exponent+1));
    float sinTheta =
            std::sqrt(std::max((float)0., (float)1. - cosTheta * cosTheta));
    wh = Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi),
                  cosTheta);
    if (!SameHemisphere(wo, wh)) wh = -wh;

    return wh;
}

CustomDistribution::CustomDistribution(float rough)
{
    if(rough>10000.f)
        rough=10000.f;
    roughness=rough;
}
float CustomDistribution::D(const Vector3f &wh) const
{
    float costhetah = AbsCosTheta(wh);
    return (roughness+2) * Inv2Pi * pow(costhetah, roughness);
}
Vector3f CustomDistribution::Sample_wh(const Vector3f &wo, const Point2f &xi) const
{
    Vector3f wh;
    float cosTheta = 0, phi = (2 * Pi) * xi[1];
    float tanTheta2 = roughness * roughness * xi[0] / (1.0f - xi[0]);
    cosTheta = 1 / std::sqrt(1 + tanTheta2);
    float sinTheta = std::sqrt(std::max((float)0., (float)1. - cosTheta * cosTheta));

    wh = Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi),
                  cosTheta);
    if (!SameHemisphere(wo, wh)) wh = -wh;

    return wh;

}
float CustomDistribution::Lambda(const Vector3f &w) const
{
    float absTanTheta = std::abs(TanTheta(w));
    if (std::isinf(absTanTheta)) return 0.;

    // Compute alpha for direction w
    float alpha =
            std::sqrt((Cos2Phi(w)+ Sin2Phi(w)) * roughness*roughness);
    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1 + std::sqrt(1.f + alpha2Tan2Theta)) / 2;

}
