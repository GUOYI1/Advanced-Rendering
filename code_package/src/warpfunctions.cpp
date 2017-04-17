#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>

Point3f WarpFunctions::squareToDiskUniform(const Point2f &sample)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented uniform disk warping!");
    float r=std::sqrt(sample.x),theta=TwoPi*sample.y;
    if(theta<0)
        theta=0;
    if(theta>TwoPi)
        theta=TwoPi;
    return Point3f(r*cos(theta),r*sin(theta),0);
//    return Point3f();
}

Point3f WarpFunctions::squareToDiskConcentric(const Point2f &sample)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented concentric disk warping!");
    float a=2*sample.x-1, b=2*sample.y-1;
    float r=0, theta=0;
    if(a>-b)
    {
        if(a>b)
        {
            r=a;
            theta=PiOver4*b/a;
        }
        else
        {
            r=b;
            theta=PiOver4*(2-a/b);
        }
    }
    else
    {
        if(a<b)
        {
            r=-a;
            theta=PiOver4*(4+b/a);
        }
        else
        {
            r=-b;
            if(b!=0)
                theta=PiOver4*(6-a/b);
            else
                theta=0;
        }

    }
    return Point3f(r*cos(theta),r*sin(theta),0);
}

float WarpFunctions::squareToDiskPDF(const Point3f &sample)
{
    //TODO
    return InvPi;
}

Point3f WarpFunctions::squareToSphereUniform(const Point2f &sample)
{
    //TODO
    float z=1-2*sample.y;
    float x=cos(TwoPi*sample.x)*std::sqrt(1-z*z);
    float y=sin(TwoPi*sample.x)*std::sqrt(1-z*z);
    return Point3f(x,y,z);
}

float WarpFunctions::squareToSphereUniformPDF(const Point3f &sample)
{
    //TODO
    return Inv4Pi;
}

Point3f WarpFunctions::squareToSphereCapUniform(const Point2f &sample, float thetaMin)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented sphere cap warping!");
    //float z=1-2*sample.y*(Pi-glm::radians(thetaMin))/Pi;
    float z=1-2*(sample.y*(1-cos(Pi-glm::radians(thetaMin)))+cos(Pi-glm::radians(thetaMin)));
    //float z=sample.y*(Pi-glm::radians(thetaMin))/Pi_2;
    float x=cos(TwoPi*sample.x)*std::sqrt(1-z*z);
    float y=sin(TwoPi*sample.x)*std::sqrt(1-z*z);
    return Point3f(x,y,z);

}

float WarpFunctions::squareToSphereCapUniformPDF(const Point3f &sample, float thetaMin)
{
    //TODO
    float area=(1-cos(Pi-glm::radians(thetaMin)))*TwoPi;
    return 1/area;
}

Point3f WarpFunctions::squareToHemisphereUniform(const Point2f &sample)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented uniform hemisphere warping!");
    float z=sample.y;
    float x=cos(TwoPi*sample.x)*std::sqrt(1-z*z);
    float y=sin(TwoPi*sample.x)*std::sqrt(1-z*z);
    return Point3f(x,y,z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const Point3f &sample)
{
    //TODO
    return Inv2Pi;

}

Point3f WarpFunctions::squareToHemisphereCosine(const Point2f &sample)
{
    //TODO
    Point3f disk_coords=squareToDiskConcentric(sample);
    float z=std::sqrt(1-disk_coords[0]*disk_coords[0]-disk_coords[1]*disk_coords[1]);
    return Point3f(disk_coords.x,disk_coords.y,z);
}

float WarpFunctions::squareToHemisphereCosinePDF(const Point3f &sample)
{
    //TODO
    return InvPi*sample.z;
}
