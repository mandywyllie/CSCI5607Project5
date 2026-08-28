#ifndef HITTABLE_H
#define HITTABLE_H

#include <vector>
#include "PGA_3D.h"

class Ray{
    public:
        Ray()=default;
        Point3D start;
        Line3D line;
};

//our spheres and our geometric shapes are both going to be hittable objects so we want to have something that can represent both
//we want to be able to take a ray an a hittable object and store data about wether it hit, the hitpoint, etc
class HitRecord{ //where we will record the hitpoint, the normal, and the distance
    public:
        HitRecord() = default;
        Point3D hitPoint;
        Dir3D normal;
        float t;
};


class Hittable {//we will make a class hittable which has a function in it that determines wether or not a ray hits a hittable object, and returns a hitrecord
    public:
    virtual ~Hittable() = default;
    Color ambientColor;
    Color diffuseColor;
    Color specularColor;
    float phongCos;
    Color transmissiveColor;
    float indexORefract;
    bool solid;
    bool transmissive;
    bool reflective;
    virtual bool hit(Point3D rayStart, Line3D rayLine, float t_min, float t_max, HitRecord& rec)=0;//we will determine wether we hit a point based on 
};



#endif