#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class Sphere : public Hittable {
    public:
    Point3D center;
    float radius;

    public: Sphere(Point3D c, float r, Color a, Color d, Color s, float p, Color t, float i) : center(c), radius(r){
        this->ambientColor = a;
        this->diffuseColor = d;
        this->specularColor = s;
        this->phongCos = p;
        this->transmissiveColor = t;
        this->indexORefract = i;
        if (t.r<=0 & t.g <= 0 & t.b <= 0){
            this->transmissive = false;
        }else{
            this->transmissive = true;
        }
        if (s.r<=0 & s.g <= 0 & s.b <= 0){
            this->reflective = false;
        }else{
            this->reflective = true;
        }
        //spheres are solid
        this->solid = true;
    }
    
    bool hit(Point3D rayStart, Line3D rayLine, float t_min, float t_max, HitRecord& rec){
        Dir3D dir = rayLine.dir();
        float a = dot(dir,dir); // - Understand: What do we know about "a" if "rayLine" is normalized on creation?
        Dir3D toStart = (rayStart - center);
        float b = 2 * dot(dir,toStart);
        float c = dot(toStart,toStart) - radius*radius;
        float discr = b*b - 4*a*c;
        if (discr < 0){ return false;
        }else{
            float t_hit;
            float t0 = (-b + sqrt(discr))/(2*a);
            float t1 = (-b - sqrt(discr))/(2*a);

            if(t0>0.01|| t1 > 0.01){
                if (t0 > 0){
                    t_hit = t0;
                }
                if (t1>0){
                    t_hit = std::min(t_hit,t1);
                }
                rec.t = t_hit;
                rec.hitPoint = rayStart + t_hit * dir;
                rec.normal = (rec.hitPoint-center);
                return true;
            }
        }
        return false;
    }

    
};


#endif