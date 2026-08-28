#ifndef RAY_TRACE_H
#define RAY_TRACE_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // For fopen and sscanf
#define _USE_MATH_DEFINES 
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

//external includes
#include <chrono>//High resolution timer
#include <tuple>

//internal includes
#include "image_lib.h" //Defines an image class and a color class
#include "PGA_3D.h" //#3D PGA
#include "parse_pga.h" //Scene file parser
//hittable class and hittable objects (spherers and planes)
#include "hittable.h"
#include "sphere.h"
#include "triangle.h"
#include "sdf.h"

Color ApplyLightingModel(Point3D rayStart, Line3D rayLine, Point3D hitPoint, Dir3D hitNormal, int depth, std::shared_ptr<Hittable> hitObj, std::vector<std::shared_ptr<Hittable>> inObjs);
Color EvaluateRayTree(Point3D rayStart, Line3D rayLine, int depth, std::vector<std::shared_ptr<Hittable>> inObjs);

bool FindIntersection(Point3D rayStart, Line3D rayLine, Point3D& hitPoint, Dir3D& normal, std::shared_ptr<Hittable>& hitObj) {
    HitRecord closestRec;
    float closest_t = 1e10f;
    bool hit_any = false;
    std::shared_ptr<Hittable> closestObj = nullptr;

    for (const auto& obj : sceneObjects) {
        HitRecord tempRec;
        // std::cout << (obj->ambientColor).b << "\n";
        if (obj->hit(rayStart, rayLine, 0.01f, closest_t, tempRec)) {
            hit_any = true;
            if (tempRec.t < closest_t){
                closest_t = tempRec.t;
                closestRec = tempRec;
                closestObj = obj;
            }
        }
    }

    if (hit_any) {
        hitPoint = closestRec.hitPoint;
        normal = closestRec.normal;
        hitObj = closestObj;

        return true;
    }

    return false;
}

Color DiffuseAndSpecular(std::shared_ptr<Light> l, Point3D rayStart, Line3D rayLine, Point3D hitPoint, Dir3D hitNormal, Plane3D hitPlane, std::shared_ptr<Hittable> hitObj){
    Dir3D lightLine = l->getLightLine(hitPoint);
    Dir3D lightnormalized = lightLine.normalized();
    Dir3D normal = hitNormal.normalized();
    Dir3D viewDir = (rayStart - hitPoint).normalized();

    //for diffuse
    float cosTheta = std::max(0.0f,dot(lightnormalized,normal));
    
    //reflection for specular
    Dir3D reflectedLightRay = sandwhich(hitPlane, lightnormalized).normalized(); 

    Color I = l->getIL(lightLine);

    Color LD=(hitObj->diffuseColor)*I*cosTheta;

    float specDot = std::max(0.0f, -dot(viewDir, reflectedLightRay)); //note that -dot is the correct version
    Color LS = (hitObj->specularColor) * I * std::pow(specDot, (hitObj->phongCos));

    return LD+LS;
}

float getCurrentIOR(const std::vector<std::shared_ptr<Hittable>>& insideObjects) {
    if (insideObjects.empty()) {
        return 1.0f; //air
    }
    //return IOR of the last entered object currently containing the ray point
    return insideObjects.back()->indexORefract; 
}

//takes a ray oriented towards a surface and returns the ray refracted through the surface (note that the code would be different if we were using a ray that was shot out of the surface)
Dir3D Refract(MultiVector ray, Dir3D hitNormal, std::shared_ptr<Hittable> hitObj, std::vector<std::shared_ptr<Hittable>>& inObjs){
    Dir3D rayDir = Line3D(ray).dir();
            //if not-> return the same ray (don't refract through a thin object)
    if (!hitObj->solid){
        return rayDir;
    }
    //ni initial index of refraction, index of refraction of inObj
    float ni;
    //nr index of refraction of the material the ray is entering
    float nr;

    if (inObjs.empty()){//we are not inside anything, so we are going from air to inside the hitobject
        ni = 1;
        nr = hitObj->indexORefract;
        inObjs.push_back(hitObj);
    }else{
        ni = getCurrentIOR(inObjs);
        // Check if we are exiting hitObj by seeing if it's already in inObjs
        auto it = std::find(inObjs.begin(), inObjs.end(), hitObj); //returns inObjs.end() if its not in the list
        bool isExiting = (it != inObjs.end()); //true if it is in the list, false otherwise
        if (isExiting){
            inObjs.erase(it);//erase that instance of hitObj from the inObjs heirarchy
            //then our initial is inObj and we are entering the last object in inObjs
        }else{
            inObjs.push_back(hitObj);
        }
        nr = getCurrentIOR(inObjs); //if we were exiting hitObj, we removed it so this gives the ior of the last object before it
        //if we were entering hitObj, we added it to the list so this gives us the ior of hitObj
    }

    //thetai - angle between the initial ray and the surface normal, need to compute
    Dir3D rayDirNormalized = (rayDir).normalized();
    Dir3D normalNormalized = hitNormal.normalized();
    float cosThetai = dot(((-1.0f)*rayDirNormalized), normalNormalized); //note that rayDir is inverted to point in the same direction as the normal (out from hitpoint)
    
    if (cosThetai < 0.0f) {
        normalNormalized = (-1.0f)*normalNormalized;            //Flip normal if we are inside
        cosThetai = -cosThetai; //Make cosine positive
    }
    
    cosThetai = std::max(-1.0f, std::min(1.0f, cosThetai)); //clamp
    // float thetai = std::acos(cosThetai); //(in radians)

    //only compute ni/nr once
    float ninr = ni/nr;
    //thetar - angle between refracted ray and the negated normal, compute using snells law and thetai
    float sin2_r = ninr * ninr * (1.0f - cosThetai * cosThetai);
    
    //if there is total internal reflection, return zero direction
    if (sin2_r > 1.0f) {
        return Dir3D(0, 0, 0); 
    }
    
    float cosThetar = std::sqrt(1.0f - sin2_r);

    // float thetar = std::asin((ninr)*std::sin(thetai));

    //ray - initial ray
    //refractedRay - ray after passing through material
    Dir3D refractedRay = ((ninr*cosThetai)-cosThetar)*normalNormalized+(ninr*rayDirNormalized);

    return refractedRay;
}

Color ApplyLightingModel(Point3D rayStart, Line3D rayLine, Point3D hitPoint, Dir3D hitNormal, int depth, std::shared_ptr<Hittable> hitObj, std::vector<std::shared_ptr<Hittable>> inObjs){
    Color contribution = Color(0,0,0);
    Line3D hitLine = vee(hitNormal.normalized(), hitPoint);
    Plane3D hitPlane = dot(hitLine, hitPoint);
    float offSet = 0.005f;

    //for each light
    for (const auto& light :lights){
        //if the light is not blocked
        //get line from hitpoint to light source
        Dir3D lightLine = light->getLightLine(hitPoint);
        float lightDist = lightLine.magnitude();
        Dir3D shadowDir = lightLine.normalized();

        Point3D shadowStart = hitPoint+(shadowDir*offSet);
        Line3D shadowLine = (vee(shadowStart,shadowDir)).normalized();

        Dir3D shadowNormal;
        Point3D shadowHitPoint;
        std::shared_ptr<Hittable> blockingObj = nullptr;
        bool blocked = FindIntersection(shadowStart,shadowLine, shadowHitPoint, shadowNormal, blockingObj);
        Color shadowFactor = Color(1.0f, 1.0f, 1.0f);
        // distance along the shadow ray from the hit point to the intersection point with an object = distance from shadowHitPoint to hitPoint
        //check if this is less than the distance from the distance to the light source
        if (blocked){
            Dir3D hitToShadowHit = shadowHitPoint- shadowStart;
            float hitDist = hitToShadowHit.magnitude();

            //for a directional light, any blocking object will cast a shadow, otherwise we see if the blocking object is closer than the light
            if(std::dynamic_pointer_cast<DirLight>(light)|| hitDist <lightDist){
                continue;
                // if (!blockingObj->transmissive) {
                //     continue; 
                // }
                // //if it is transmissive, scale light intensity by the object's transmissive color
                // shadowFactor = blockingObj->transmissiveColor;
            }
        }

        Color lightContrib = DiffuseAndSpecular(light, rayStart,rayLine, hitPoint,hitNormal, hitPlane, hitObj);
        contribution = contribution + (lightContrib * shadowFactor);

    }
    if (depth <= maxDepth){
        if (hitObj->reflective){
            //mirror
            Line3D mirrorLine = sandwhich(hitPlane, rayLine);
            Dir3D mirrorDir = mirrorLine.dir();
            Point3D mirrorStart = hitPoint + (mirrorDir*offSet); //Start the mirror ray slightly away from the hitpoint to avoid getting a "reflection" from the object itself
            contribution =contribution+ (hitObj->specularColor)*EvaluateRayTree(mirrorStart,mirrorLine, depth+1, inObjs); //kr = specularColor
        }

        if (hitObj->transmissive){
            //refraction
            Dir3D refractedRayDir = Refract(rayLine, hitNormal, hitObj, inObjs); //Refract() should give a ray that is refracted through the object so you can "see through" the object 
            float d=refractedRayDir.magnitude();   
            if ((d*d)>1e-6f){//don't refract if we have total internal reflection
                Point3D refractStart = hitPoint + (refractedRayDir * offSet);//Start the refract ray slightly away from the hitpoint to avoid seeing the object through itself 
                //then we get the color of the next object that is hit, and scale it by the transmissiveness of each channel, i.e. the transmissive color
                Line3D refractedRayLine = vee(refractStart,refractedRayDir).normalized();
                contribution = contribution + (hitObj->transmissiveColor)*EvaluateRayTree( refractStart, refractedRayLine, depth+1, inObjs);
            }
        }
    }


    // contribution += Emission(hit); //for area lights only

    Color LA = (hitObj->ambientColor)*ambientLight;

    contribution = contribution + LA;
    return contribution;
}

Color EvaluateRayTree(Point3D rayStart, Line3D rayLine, int depth, std::vector<std::shared_ptr<Hittable>> inObjs){
        bool hit_something;
        Point3D hitPoint;
        Dir3D normal; //hitpoint and normal
        std::shared_ptr<Hittable> hitObj = nullptr;
        hit_something = FindIntersection(rayStart, rayLine, hitPoint, normal, hitObj); //will return a true if hit something and false otherwise and update the variables hitpoint and normal if yes
        if(hit_something){
            return ApplyLightingModel(rayStart,rayLine,hitPoint, normal, depth, hitObj, inObjs);
        }else{
            return backgroundColor;//set in parse header file
        }
}


Image RayCast(int imgW, int imgH){
    Image outputImg = Image(imgW,imgH);
    float halfW = imgW/2, halfH = imgH/2;
    float d = halfH / tanf(halfAngleVFOV * (M_PI / 180.0f));
    for (int i = 0; i < imgW; i++){
        for (int j = 0; j < imgH; j++){
        float u = (halfW - (imgW)*((i+0.5)/imgW));
        float v = (halfH - (imgH)*((j+0.5)/imgH));
        Point3D p = eye - d*forward + u*right + v*up;
        Dir3D rayDir = (p - eye); 
        Line3D rayLine = vee(eye,rayDir).normalized();  //Normalizing here is optional
        
        std::vector<std::shared_ptr<Hittable>> inObjs;
        Color color = EvaluateRayTree(eye,rayLine,0, inObjs);
        // color.r = std::min(1.0f, std::max(0.0f, color.r));
        // color.g = std::min(1.0f, std::max(0.0f, color.g));
        // color.b = std::min(1.0f, std::max(0.0f, color.b));
        outputImg.setPixel(i,j, color);
        //outputImg.setPixel(i,j, Color(fabs(i/imgW),fabs(j/imgH),fabs(0))); 
    }
  }
  return outputImg;
}

#endif