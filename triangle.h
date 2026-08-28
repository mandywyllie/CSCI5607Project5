#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"

extern std::vector<Point3D> vertices;
extern std::vector<Dir3D> normals;

class Triangle : public Hittable {
    public:
    int v1;
    int v2;
    int v3;
    int n1;
    int n2;
    int n3;
    bool simple; //keeps track of wether we need to calculate the norm at each point or if its just the norm everywhere
    Dir3D normAll; //norm for simple triangles

    //constructor for simple triangles
    public: Triangle(int vert1, int vert2, int vert3, Color a, Color d, Color s, float p, Color t, float i) : v1(vert1), v2(vert2), v3(vert3){
        this->ambientColor = a;
        this->diffuseColor = d;
        this->specularColor = s;
        this->phongCos = p;
        this->transmissiveColor = t;
        this->indexORefract = i;
        this->simple = true;
        calculateNormAll();
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
        //triangles are not solid
        this->solid = false;

    }

    //constructor for nonsimple triangles
    public: Triangle(int vert1, int vert2, int vert3, int norm1, int norm2, int norm3, Color a, Color d, Color s, float p, Color t, float i) : v1(vert1), v2(vert2), v3(vert3), n1(norm1), n2(norm2), n3(norm3){
        this->ambientColor = a;
        this->diffuseColor = d;
        this->specularColor = s;
        this->phongCos = p;
        this->transmissiveColor = t;
        this->indexORefract = i;
        this->simple = false;
    }
    
    public: void calculateNormAll(){
        Point3D p1 = vertices[v1];
        Point3D p2 = vertices[v2];
        Point3D p3 = vertices[v3];
    
        Dir3D edge1 = p2 - p1;
        Dir3D edge2 = p3 - p1;
    
        normAll = cross(edge1, edge2).normalized();    
    }


    bool hit(Point3D rayStart, Line3D rayLine, float t_min, float t_max, HitRecord& rec){
        Dir3D dir = rayLine.dir();

        Point3D& p1 = vertices[v1];
        Point3D& p2 = vertices[v2];
        Point3D& p3 = vertices[v3];

        Dir3D edge1 = p2 - p1;
        Dir3D edge2 = p3 - p1;
        Dir3D h = cross(dir, edge2);
        float a = dot(edge1, h);    

        //if ray is parallel to the plane, it will never intersect it:
        if (std::abs(a) < 1e-8f) return false;

        //then ray is not parallel to the plane, so we find where it intersects
        //check if the ray is in the triangle using barycentric coordinates u,v,w
        float f = 1.0f / a;
        Dir3D s = rayStart - p1;
        float u = f * dot(s, h);

        if (u < 0.0f || u > 1.0f) return false;

        Dir3D q = cross(s, edge1);
        float v = f * dot(dir, q);

        if (v < 0.0f || (u + v) > 1.0f) return false;

        //ray must intersect triangle, so we update hitdata
        float t = f * dot(edge2, q);
        if (t < t_min || t > t_max) return false;
        rec.t = t;
        rec.hitPoint = rayStart + (dir * t);

        //compute normal
        if (simple) {
            rec.normal = normAll;
        } else { //we need to interpolate
            float w = 1.0f - u - v;
            Dir3D interpolatedNormal = (w * normals[n1]) + (u * normals[n2]) + (v * normals[n3]);
            rec.normal = interpolatedNormal.normalized();
        }

        if (dot(dir, rec.normal) > 0.0f) {
            rec.normal = -1.0f * rec.normal;
        }

        return true;
    }

    
};


#endif