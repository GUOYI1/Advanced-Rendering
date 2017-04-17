#include "squareplane.h"

float SquarePlane::Area() const
{
    //TODO
    Vector3f scale=transform.getScale();
    //Since this is a 2D geometry, we should not consider the scale along z axis.
    return 1*scale.x*scale.y;
}

bool SquarePlane::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the square
    if(t > 0 && P.x >= -0.5f && P.x <= 0.5f && P.y >= -0.5f && P.y <= 0.5f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void SquarePlane::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    //TODO: Compute tangent and bitangent
    *tan=glm::normalize(transform.T3()*Point3f(1,0,0));
    *bit=glm::normalize(glm::cross(*nor,*tan));
}


Point2f SquarePlane::GetUVCoordinates(const Point3f &point) const
{
    return Point2f(point.x + 0.5f, point.y + 0.5f);
}
Intersection SquarePlane::Sample(const Point2f &xi, Float *pdf) const
{
    Intersection I;
    Point3f p(xi[0]-0.5f,xi[1]-0.5f,0);
    *pdf=1/Area();
    InitializeIntersection(&I,-1.0f,p);
    return I;
}
Bounds3f SquarePlane::WorldBound() const
{
    Bounds3f box(Point3f(-0.5f,-0.5f,0),Point3f(0.5f,0.5f,0));
    return box.Apply(transform);
}
