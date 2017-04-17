#include"inversesphere.h"
void InverseSphere::ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const
{
    *nor = -glm::normalize(transform.invTransT() * glm::normalize(P));
    //TODO: Compute tangent and bitangent
    //DELETEME VVVVVV
    *tan = glm::normalize(transform.T3() * glm::cross(Vector3f(0,1,0), (glm::normalize(P))));
    *bit = glm::normalize(glm::cross(*nor, *tan));
}
