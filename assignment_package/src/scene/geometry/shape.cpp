#include "shape.h"
#include <QDateTime>

pcg32 Drawable::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());


void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    //TODO
    Intersection I;
    I=Sample(xi,pdf);
    Vector3f R=I.point-ref.point;
    Float R_L=glm::length2(R);
    Float cos_value=AbsDot(glm::normalize(-R),I.normalGeometric);
    if(cos_value==0)
        *pdf=0;
    *pdf*=R_L/cos_value;
    return I;
//    return Intersection();
}
