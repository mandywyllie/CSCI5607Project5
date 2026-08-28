#ifndef LIGHT_H
#define LIGHT_H

#include "PGA_3D.h"

class Light{
    public:
    Color intensity;
    Point3D loc;
    Light(Color i, Point3D l):intensity(i), loc(l){
    }
    virtual ~Light() = default;
    virtual Dir3D getLightLine(Point3D p) const = 0;
    virtual Color getIL(Dir3D lightLine) const = 0;
};

class DirLight:public Light{
    public:
    Dir3D dir;
    DirLight(Color i, float x, float y, float z):Light(i,Point3D(x,y,z)), dir(Dir3D(x,y,z).normalized()){
    }
    Dir3D getLightLine (Point3D p) const override{
        return -1.0f*dir;
    }

    Color getIL(Dir3D lightLine) const override {
        return intensity;
    }
};

class PointLight:public Light{
    public:
    PointLight(Color i, Point3D l):Light(i,l){
    }
    Dir3D getLightLine (Point3D p) const override{
        Point3D lightLoc = loc;
        Point3D hitP = p;
        return (lightLoc - hitP);
    }
    Color getIL(Dir3D lightLine) const override {
        float d = lightLine.magnitude(); //dist to hit point
        float distSquared = d*d;
        distSquared = std::max(1.0f, distSquared); 
        if (distSquared < 1e-6f) {
            return intensity;
        }
        return intensity * (1.0f / distSquared);
    }
};

class SpotLight:public Light{
    public:
    Dir3D dir;
    float cosAngle1; 
    float cosAngle2; 

    SpotLight(Color i, Point3D l, Dir3D d, float a1_deg, float a2_deg): Light(i, l), dir(d.normalized()) {
        constexpr float degToRad = 3.14159265f / 180.0f;
        cosAngle1 = std::cos((a1_deg) * degToRad);
        cosAngle2 = std::cos((a2_deg) * degToRad);
    }

    Dir3D getLightLine (Point3D p) const override{
        Point3D lightLoc = loc;
        Point3D hitP = p;
        return (lightLoc - hitP);
    }
    Color getIL(Dir3D lightLine) const override {
        Dir3D lightToPoint = (-1.0f*lightLine).normalized();
        float cosAlpha = dot(dir, lightToPoint);

        //Outside outer cone gets no light
        if (cosAlpha <= cosAngle2) {
            return Color(0, 0, 0); //
        }

        //inside inner cone behaves like point light
        float d = lightLine.magnitude();
        float distSquared = d*d;
        if (distSquared < 1e-6f) return intensity; // if we are this close we must be in the inner cone or we are so close it doesnt matter
        Color pointLightIntensity = intensity * (1.0f / distSquared);

        if (cosAlpha >= cosAngle1) { //inside inner cone
            return pointLightIntensity;
        }

        //otherwise we are between the cones, so we have smooth linear falloff
        float falloff = (cosAlpha - cosAngle2) / (cosAngle1 - cosAngle2);
        return pointLightIntensity * falloff;    
    }
};

#endif