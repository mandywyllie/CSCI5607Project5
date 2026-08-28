#ifndef SDF_H
#define SDF_H

#include "hittable.h"
class SDFObj : public Hittable {
public:
    // Motor3D rotator;
    // Point3D center;
    SDFObj() = default; //default constructor so we can have types of sdf objects (like SDFUnion) that use a different constructor than the one below
    SDFObj(Color a, Color d, Color s, float p, Color t, float i){
        this->ambientColor = a;
        this->diffuseColor = d;
        this->specularColor = s;
        this->phongCos = p;
        this->transmissiveColor = t;
        this->indexORefract = i;
        this->transmissive = !(t.r <= 0 && t.g <= 0 && t.b <= 0);
        this->reflective = !(s.r <= 0 && s.g <= 0 && s.b <= 0);
    }

    virtual float distance(const Point3D& p) const = 0;

    // Point3D worldToLocal(Point3D p){
    //     //translated point is p-center
    //     //rotate by applying rotator
    //     return rotator(p-(p-center).magnitude());
    // }

    Dir3D getNormal(Point3D p) const {//use the gradient to get normal at the point
        const float eps = 0.001f;
        float d = distance(p);
        float nx = distance(Point3D(p.x + eps, p.y, p.z)) - d;
        float ny = distance(Point3D(p.x, p.y + eps, p.z)) - d;
        float nz = distance(Point3D(p.x, p.y, p.z + eps)) - d;
        return Dir3D(nx, ny, nz).normalized();
    }

    bool hit(Point3D rayStart, Line3D rayLine, float tMin, float tMax, HitRecord& rec) override {
        Dir3D rayDir = rayLine.dir().normalized();
        
        // //if we start at a negative distance, this means that our ray is starting inside the object
        // float initialDist = distance(rayStart);
        // bool isInside = (initialDist < 0.0f);
        
        //offset to prevent self-intersection
        float t = tMin > 0.0f ? tMin : 0.005f;
        const int maxSteps = 128;
        const float hitThreshold = 0.001f;

        for (int i = 0; i < maxSteps; ++i) {
            Point3D currentPos = rayStart + (rayDir * t);
            
            //if we are inside the object we need to make the distance positive for the rest of our math
            float rawDist = distance(currentPos);
            if (std::isnan(rawDist)) return false;
            float stepDist = std::abs(rawDist);
            
            if (stepDist < hitThreshold) {
                rec.t = t;
                rec.hitPoint = currentPos;
                
                //if we are inside, we need to flip the normal so that it points in the same direction as the rayLine
                Dir3D outwardNormal = getNormal(currentPos);
                rec.normal = (rawDist < 0.0f) ? -1.0f * outwardNormal : outwardNormal;
                return true;
            }

            t += stepDist;
            if (t >= tMax) break;
        }
        return false;
    }

};

class SDFSphere:public SDFObj{
    public:
    Point3D center;
    float radius;

    SDFSphere(Point3D c, float r, Color a, Color d, Color s, float p, Color t, float i) : SDFObj(a,d,s,p,t,i), center(c), radius(r){
        //spheres are solid
        this->solid = true;
    }

    float distance(const Point3D& p)const override{
        Dir3D difference = p - center;
        return difference.magnitude() - radius;
    }

};

class SDFTorus:public SDFObj{
    public:
    float r1;
    float r2;
    Motor3D worldToLocal;

    SDFTorus(const Point3D& c, float r_1, float r_2, const Dir3D& up, Color a, Color d, Color s, float p, Color t, float i) : SDFObj(a,d,s,p,t,i), r1(r_1), r2(r_2){
        //tori are solid
        this->solid = true;
        Dir3D validUp = (up.magnitudeSqr() > 1e-6f) ? up : Dir3D(0, 1, 0);
        this->worldToLocal = getWorldToLocal(c, validUp);   
        // std::cout <<  worldToLocal << "\n";
        // std::cout << worldToLocal(Point3D(0, 0, 0)) << "\n";
    }

    float distance(const Point3D& p)const override{
        //first move to the center of the taurus
        Point3D localP = worldToLocal(p);
        //get the distance from p to localP
        //float dToLocalP = (localP-p).magnitude();

        //distance from p to the center of the taurus along the xz plane
        float xzDist = std::sqrt(localP.x * localP.x + localP.z * localP.z);

        //get to the bigger circle (r1)
        float qx = xzDist - r1;
        float qy = localP.y;

        //get the distance to the center of the second circle along the xy plane, then to then to the smaller circle (r2)
        return std::sqrt(qx * qx + qy * qy) - r2;
    }
};


class SDFBox:public SDFObj{
    public:
    Dir3D halfExtents;
    Motor3D worldToLocal;

    SDFBox(const Point3D& c, const Dir3D& b,const Dir3D& up, Color a, Color d, Color s, float p, Color t, float i) : SDFObj(a,d,s,p,t,i), halfExtents(b){
        //boxes are solid
        this->solid = true;
        Dir3D validUp = (up.magnitudeSqr() > 1e-6f) ? up : Dir3D(0, 1, 0);
        this->worldToLocal = getWorldToLocal(c, validUp);
    }

    float distance(const Point3D& p)const override{
        //move to the box local (where the box is centered at the origin and oriented along the coordinate axes)
        Point3D localP = worldToLocal(p);
        if (std::isnan(localP.x)) std::cout << "NaN detected in worldToLocal!\n";
        //get the distance needed to travel to each component
        float qx = std::abs(localP.x) - halfExtents.x;
        float qy = std::abs(localP.y) - halfExtents.y;
        float qz = std::abs(localP.z) - halfExtents.z;

        //for points outside the box:
        float maxQx = std::max(qx, 0.0f);
        float maxQy = std::max(qy, 0.0f);
        float maxQz = std::max(qz, 0.0f);
        float exterior = std::sqrt(maxQx * maxQx + maxQy * maxQy + maxQz * maxQz);

        //for points inside the box:
        float interior = std::min(std::max(qx, std::max(qy, qz)), 0.0f);

        return exterior + interior;
    }
};

//TODO make sdfunion with hard minimum

//TODO rename to be sdfsmoothunion
//the union of two sdf objects at the same point in space
class SDFUnion:public SDFObj{
public:
    std::shared_ptr<SDFObj> a;
    std::shared_ptr<SDFObj> b;
    float k;

    //makes a union object of the two objects
    SDFUnion(std::shared_ptr<SDFObj> objA, std::shared_ptr<SDFObj> objB, float blendFactor = .3): a(objA), b(objB), k(blendFactor){
        this->ambientColor = objA->ambientColor;
        this->diffuseColor = objA->diffuseColor;
        this->specularColor = objA->specularColor;
        this->phongCos = objA->phongCos;
        this->transmissiveColor = objA->transmissiveColor;
        this->indexORefract = objA->indexORefract;
        this->transmissive = objA->transmissive;
        this->reflective = objA->reflective;

    }

    //for the distance we will take the smooth minimum of the distance to the two objects
    float distance(const Point3D& p)const override{
        float dA = a->distance(p);
        float dB = b->distance(p);

        //Inigo Quilez cubic smooth minimum
        float h = std::max(k - std::abs(dA - dB), 0.0f) / k;
        return std::min(dA, dB) - h * h * h * k * (1.0f / 6.0f);
    }

    //when we calculate hit, interpolate colors based on how close the hitpoint is to each object
    //Commented out because it gets really messed up in tori and some other cases
    // bool hit(Point3D rayStart, Line3D rayLine, float tMin, float tMax, HitRecord& rec) override {
    //     //calculate wether it hit and get the hitPoint using the distance defined above
    //     bool didHit = SDFObj::hit(rayStart, rayLine, tMin, tMax, rec);

    //     if (didHit) {
    //         float dA = a->distance(rec.hitPoint); //distance from hitpoint to A
    //         float dB = b->distance(rec.hitPoint); //distance from hitpoint to B

    //         //interpolate colors based on how close the hitpoint is to each object
    //         float factor = std::max(0.0f, std::min(1.0f, 0.5f + 0.5f * (dB - dA) / k));
            
    //         //set the colors of the union object to be these interpolated colors 
    //         //(when we call hit on it again with a different ray, it will change these colors)
    //         this->ambientColor  = a->ambientColor  * factor + b->ambientColor  * (1.0f - factor);
    //         this->diffuseColor  = a->diffuseColor  * factor + b->diffuseColor  * (1.0f - factor);
    //         this->specularColor = a->specularColor * factor + b->specularColor * (1.0f - factor);
    //         this->phongCos      = a->phongCos      * factor + b->phongCos      * (1.0f - factor);
    //     }

    //     return didHit;
    // }

};

class SDFMorph:public SDFObj{
    public:
        std::shared_ptr<SDFObj> a;
        std::shared_ptr<SDFObj> b;
        float t;
    
        //morphs one object into another, t must be between 0 and 1
        SDFMorph(std::shared_ptr<SDFObj> objA, std::shared_ptr<SDFObj> objB, float time = .5): a(objA), b(objB), t(time){
            //if one of them is transmissive, the morph will be transmissive
            this->transmissive = objA->transmissive || objB->transmissive;
            this->reflective = objA->reflective || objB->reflective;
            //interpolate the color
            this->ambientColor = a->ambientColor*t + b->ambientColor*(1.0f - t);
            this->diffuseColor = a->diffuseColor*t + b->diffuseColor*(1.0f - t);
            this->specularColor = a->specularColor*t + b->specularColor*(1.0f - t);
            this->phongCos = a->phongCos*t + b->phongCos*(1.0f - t);
            this->transmissiveColor = objA->transmissiveColor*t + b->transmissiveColor*(1.0f - t);
            this->indexORefract = objA->indexORefract*t + b->indexORefract*(1.0f - t);
        }
    
        //interpolate the distance
        float distance(const Point3D& p)const override{
            float dA = a->distance(p);
            float dB = b->distance(p);
            return dA*t + dB*(1-t); 
        }    
    };
    
    
    
    



#endif