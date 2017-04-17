#include<scene/geometry/implicit_surface.h>
#include<scene/geometry/cube.h>
#define EPSILON 0.0001
#define START 0.0
#define STOP 200.0
#define MAX_STEPS 500
#define TAU 6.28318530718
float TorusSDF(Point3f isect_test, Vector2f dimensions)
{
    Vector2f v1(isect_test.x,isect_test.z);
    Vector2f v2(glm::length(v1)-dimensions.x,isect_test.y);
    return glm::length(v2)-dimensions.y;
}
float TwistTorusSDF(Point3f isect_test)
{
    float c = cos(15.0f* sin(0.4)*isect_test.y);
    float s = sin(15.0f*sin(0.4)*isect_test.y);
    glm::mat2 m(c,-s,s,c);
    Point2f xz(isect_test.x,isect_test.z);
    Point3f q = Point3f(m*xz,isect_test.y);
    return TorusSDF(q,Vector2f(0.6f,0.23f));

}

float HeartSDF(Point3f isect_test)
{

    Vector3f pp = isect_test * isect_test;
    Vector3f ppp = pp * isect_test;
    float a = pp.x + 2.25 * pp.y + pp.z - 1.0f;
    float result=a * a * a - (pp.x + 0.1125 * pp.y) * ppp.z;
    if(result<0)
        result=-pow(-result,1.0f/6.0f);
    else
        result=pow(result,1.0f/6.0f);
    return result;

}
float ToothSDF(Point3f isect_test)
{

    float x4=pow(isect_test.x,4);
    float y4=pow(isect_test.y,4);
    float z4=pow(isect_test.z,4);
    float x2=pow(isect_test.x,2);
    float y2=pow(isect_test.y,2);
    float z2=pow(isect_test.z,2);
    float result=(x4+y4+z4)-(x2+y2+z2);
    if(result<0)
        result=-pow(-result,1.0f/6.0f);
    else
        result=pow(result,1.0f/6.0f);
    return result;

}

float SceneSDF(Point3f isect_test,SurfaceType type)
{
    float result=1.0f;
    switch(type)
    {
    case Torus:
        result=TorusSDF(isect_test,Vector2f(1.2f,0.5f));
        break;
    case TwistTorus:
        result=TwistTorusSDF(isect_test);
        break;
    case Heart:
        result=HeartSDF(isect_test);
        break;
    case Tooth:
        result=ToothSDF(isect_test);
        break;
    }
    return result;
}
float Implicit_Surface::RayMarch(const Ray& r,float start,float stop) const
{
    float t=start;
    for(int i=0;i<MAX_STEPS;i++)
    {
        float dist=SceneSDF(r.origin+t*r.direction,type);
        if(dist<EPSILON)
        {
            if(dist<0)
            {
                for(float j=0;j<0.1f;j+=0.001f)
                {
                    t-=j;
                    float back_dist=SceneSDF(r.origin+t*r.direction,type);
                    if(back_dist>0.0f)
                        break;
                }
                if(t!=t)
                    return stop;
            }
            return t;
        }
        t+=dist;
        if(t>=stop)
            break;
    }
    return stop;
}

float Implicit_Surface::Area() const
{
    //We don't use this
    return 0;
}
bool Implicit_Surface::Intersect(const Ray &ray, Intersection *isect) const
{
    Ray r_loc=ray.GetTransformedCopy(transform.invT());
    float t=RayMarch(r_loc,START,STOP);
    if(t>=STOP-EPSILON)
        return false;
    else if(t>EPSILON)
    {
        Point3f P = glm::vec3(r_loc.origin + t*r_loc.direction);
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}
void Implicit_Surface::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor=glm::normalize(Point3f(SceneSDF(Point3f(P.x+EPSILON,P.y,P.z),type)-SceneSDF(Point3f(P.x-EPSILON,P.y,P.z),type),\
                                SceneSDF(Point3f(P.x,P.y+EPSILON,P.z),type)-SceneSDF(Point3f(P.x,P.y-EPSILON,P.z),type),\
                                SceneSDF(Point3f(P.x,P.y,P.z+EPSILON),type)-SceneSDF(Point3f(P.x,P.y,P.z-EPSILON),type)));
    *nor=glm::normalize(transform.invTransT() * (*nor));
    CoordinateSystem(*nor,tan,bit);


}
Point2f Implicit_Surface::GetUVCoordinates(const Point3f &point) const
{
    //We don't use this
    return Point2f();
}
Intersection Implicit_Surface::Sample(const Point2f &xi, Float *pdf) const
{
    //We don't use this
    return Intersection();
}
Bounds3f Implicit_Surface::WorldBound() const
{
    Bounds3f box(Point3f(-3.0f,-3.0f,-3.0f),Point3f(3.0f,3.0f,3.0f));
    return box.Apply(transform);
}


