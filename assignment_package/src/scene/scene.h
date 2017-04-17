#pragma once
#include <QList>
#include <raytracing/film.h>
#include <scene/camera.h>
#include <scene/lights/light.h>
#include <scene/geometry/shape.h>
#include "bvh.h"
#include"k-d_tree.h"

class Primitive;
class BVHAccel;
class Material;
class Light;

class Scene
{
public:
    Scene();
    ~Scene();
    QList<std::shared_ptr<Primitive>> primitives;
    QList<std::shared_ptr<Material>> materials;
    QList<std::shared_ptr<Light>> lights;

    //In the scene, there are multiple types of cameras with the same basic attributes.
    Camera camera;
    Len_Base_Camera lb_camera;
    CameraType current_camera_type;
    //Camera* current_camera; //Point to the current camera.

    Film film;

    BVHAccel* bvh;
    KdTreeAccel* kd;
    QList<std::shared_ptr<Drawable>> drawables;


    void SetCamera(const Camera &c);
    void SetLBCamera(Float lensRadius,Float focalDistance);

    void CreateTestScene();
    void Clear();

    bool Intersect(const Ray& ray, Intersection* isect) const;

    void clearBVH();
    void clearKD();
};
