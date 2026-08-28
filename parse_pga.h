
#ifndef PARSE_PGA_H
#define PARSE_PGA_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>
#include <memory>
#include "sphere.h"
// #include "triangle.h"
#include "light.h"
#include "image_lib.h"
#include "hittable.h"
#include "triangle.h"
#include "sdf.h"


//Camera & Scene Parameters (Global Variables)
//Here we set default values, override them in parseSceneFile()

//Image Parameters
int img_width = 800, img_height = 600;
std::string imgName = "untitledImage.bmp";

//Camera Parameters
Point3D eye = Point3D(0,0,0); 
Dir3D forward = Dir3D(0,0,-1).normalized();
Dir3D up = Dir3D(0,1,0).normalized();
Dir3D right = Dir3D(-1,0,0).normalized();
float halfAngleVFOV = 35; 

//Scene Parameters
std::vector<std::shared_ptr<Hittable>> sceneObjects;
Point3D spherePos = Point3D(0,0,2);
float sphereRadius = 1; 
Color ambientColor = Color(0,0,0);
Color diffuseColor = Color(1,1,1);
Color specularColor = Color(0,0,0);
float phongCos = 5;
Color transmissiveColor = Color(0,0,0);
float indexORefract = 1;
std::vector<std::shared_ptr<Light>> lights;
Color backgroundColor = Color(0,0,0);
Color ambientLight = Color(0,0,0);
int maxDepth = 5;
std::vector<Point3D> vertices;
std::vector<Dir3D> normals;

// void placeInTree(Sphere s){
//   //if bvh empty
//             //make bounding sphere that is that sphere

//   //else recursively find where it goes in tree
//   //if nonempty and distance between this sphere and all first layer bounding spheres spheres (distance between pos1 and pos2)
//   spheres.push_back(Sphere(Color(r,g,b),Point3D(x,y,z)));
// }


void parseSceneFile(std::string fileName){
  //Override the default values with new data from the file "fileName"
  std::ifstream inputFile(fileName);
  if (!inputFile.is_open()){
    std::cerr << "Error: Could not open scene file: "<< fileName << std::endl;
    return;
  }  
  
  std::string line; //initialize a variable which is a string we will call it line
  int line_number = 0; //keep track of line number

  while(std::getline(inputFile,line)){
    line_number++;
    if (line.empty()||line[0] == '#'){//skip comments or empty lines
      continue;
    }
    //if its not empty lets go through the line
    std::stringstream ss(line);
    std::string command;

    if(ss>>command){
      if (command == "sphere:") {
        float x, y, z, r;
        if (ss >> x >> y >> z >> r) { 
            spherePos = Point3D(x, y, z);
            sphereRadius = r;
            auto newSphere = std::make_shared<Sphere>(
            spherePos, sphereRadius, ambientColor, diffuseColor, specularColor, 
            phongCos, transmissiveColor, indexORefract);
            sceneObjects.push_back(newSphere);
        } else {
            std::cerr << "Error: 'sphere' command requires 4 numeric parameters." << std::endl;
        }
      }else if (command == "sdf_sphere:") {
        float x, y, z, r;
        if (ss >> x >> y >> z >> r) { 
            spherePos = Point3D(x, y, z);
            sphereRadius = r;
            auto newSphere = std::make_shared<SDFSphere>(
            spherePos, sphereRadius, ambientColor, diffuseColor, specularColor, 
            phongCos, transmissiveColor, indexORefract);
            sceneObjects.push_back(newSphere);
        } else {
            std::cerr << "Error: 'sdf_sphere' command requires 4 numeric parameters." << std::endl;
        }
      }else if (command == "sdf_box:") {
        float x, y, z, hx, hy, hz, dx, dy, dz;;
        if (ss >> x >> y >> z >> hx >> hy >> hz>> dx >> dy >> dz) { 
            auto newBox = std::make_shared<SDFBox>(
              Point3D(x, y, z), Dir3D(hx,hy,hz), Dir3D(dx,dy,dz), ambientColor, diffuseColor, specularColor, 
            phongCos, transmissiveColor, indexORefract);
            sceneObjects.push_back(newBox);
        } else {
            std::cerr << "Error: 'sdf_box' command requires 6 numeric parameters." << std::endl;
        }
      }else if (command == "sdf_torus:") {
        float x, y, z, r1, r2, dx, dy, dz; //dx, dy, dz, is the up direction
        if (ss >> x >> y >> z >> r1 >> r2 >> dx >> dy >> dz) { 
            auto newTorus = std::make_shared<SDFTorus>(
              Point3D(x, y, z), r1, r2, Dir3D(dx,dy,dz), ambientColor, diffuseColor, specularColor, 
            phongCos, transmissiveColor, indexORefract);
            sceneObjects.push_back(newTorus);
        } else {
            std::cerr << "Error: 'sdf_torus' command requires 8 numeric parameters." << std::endl;
        }
      }else if (command == "sdf_union") {
        float k;
        bool blendfactor = false;
        if (ss >> k) {
          blendfactor = true;
          // std::cout << k;
        }
        try { 
          if (sceneObjects.size() < 2) {
            throw std::runtime_error("Not enough objects for sdf_union");
          }
          auto rawB = sceneObjects.back();//store rawB in order to save it if it was passed wrong
          sceneObjects.pop_back();
          std::shared_ptr<SDFObj> B = std::dynamic_pointer_cast<SDFObj>(rawB);  
          auto rawA = sceneObjects.back(); //similarly store rawA
          sceneObjects.pop_back();
          std::shared_ptr<SDFObj> A = std::dynamic_pointer_cast<SDFObj>(rawA);
  
          //check if they are actually sdf objects
          if (!A || !B) {
              std::cerr << "Error: 'sdf_union' operands must be SDF objects." << std::endl;
              sceneObjects.push_back(rawA);
              sceneObjects.push_back(rawB);
          }else{
            auto newUn = (blendfactor) ? std::make_shared<SDFUnion>(A,B,k) : std::make_shared<SDFUnion>(A,B);
            sceneObjects.push_back(newUn);
          }
        } catch (...) {
            std::cerr << "Error: 'sdf_union' command requires two STF objects defined before it." << std::endl;
        }
      }else if (command == "sdf_morph") {
        float t;
        bool time = false;
        if (ss >> t) {
          time = true;
        }
        try { 
          if (sceneObjects.size() < 2) {
            throw std::runtime_error("Not enough objects for sdf_morph");
          }
          auto rawB = sceneObjects.back();//store rawB in order to save it if it was passed wrong
          sceneObjects.pop_back();
          std::shared_ptr<SDFObj> B = std::dynamic_pointer_cast<SDFObj>(rawB);  
          auto rawA = sceneObjects.back(); //similarly store rawA
          sceneObjects.pop_back();
          std::shared_ptr<SDFObj> A = std::dynamic_pointer_cast<SDFObj>(rawA);
  
          //check if they are actually sdf objects
          if (!A || !B) {
              std::cerr << "Error: 'sdf_union' operands must be SDF objects." << std::endl;
              sceneObjects.push_back(rawA);
              sceneObjects.push_back(rawB);
          }else{
            auto newMorph = (time) ? std::make_shared<SDFMorph>(A,B,t) : std::make_shared<SDFMorph>(A,B);
            sceneObjects.push_back(newMorph);
          }
        } catch (...) {
            std::cerr << "Error: 'sdf_morph' command requires two STF objects defined before it." << std::endl;
        }
      }else if(command == "film_resolution:"){
        float w,h;
        if (ss >> w >> h) {
          img_width=w; img_height=h;
        } else {
          std::cerr << "Error: 'image_resolution' command requires 2 numeric parameters." << std::endl;
        }
      }else if(command == "output_image:"){
        ss>> imgName;
      }else if(command == "camera_pos:"){
        float x, y, z;
        if (ss >> x >> y >> z) {
          eye = Point3D(x,y,z);
        } else {
          std::cerr << "Error: 'camera_pos' command requires 3 numeric parameters." << std::endl;
        }
      }else if(command == "camera_fwd:"){
        float x, y, z;
        if (ss >> x >> y >> z) {
          forward = Dir3D(x,y,z).normalized();
        } else {
          std::cerr << "Error: 'camera_fwd' command requires 3 numeric parameters." << std::endl;
        }
      }else if(command == "camera_up:"){
        float x, y, z;
        if (ss >> x >> y >> z) {
          up = Dir3D(x,y,z).normalized();
        } else {
          std::cerr << "Error: 'camera_up' command requires 3 numeric parameters." << std::endl;
        }
      }else if(command == "camera_fov_ha:"){
        float ha;
        if (ss >> ha) {
          halfAngleVFOV=ha;
        } else {
          std::cerr << "Error: 'camera_fov_ha' command requires 1 numeric parameters." << std::endl;
        }
      }else if(command == "background:"){
        float r,g,b;
        if (ss >> r>>g>>b) {
          backgroundColor = Color(r,g,b);
        } else {
          std::cerr << "Error." << std::endl;
        }
      }else if(command == "material:"){
        float ar,ag,ab, dr, dg, db, sr, sg, sb, ns, tr,tg, tb,ior;
        if (ss >> ar>>ag>>ab >> dr >> dg >> db >> sr >> sg >> sb >> ns >> tr >>tg >> tb >> ior) {
          ambientColor = Color(ar,ag,ab);
          diffuseColor = Color(dr,dg,db);
          specularColor = Color(sr,sg,sb);
          phongCos = ns;
          transmissiveColor = Color(tr,tg,tb);
          indexORefract = ior;
        } else {
          std::cerr << "Error." << std::endl;
        }
      }else if(command == "directional_light:"){
        float r,g,b,x,y,z;
        if (ss >> r>>g>>b>>x>>y>>z) {
          lights.push_back(std::make_shared<DirLight>(Color(r,g,b), x, y, z));
        } else {
          std::cerr << "Error." << std::endl;
        }
      }else if(command == "point_light:"){
        float r,g,b,x,y,z;
        if (ss >> r>>g>>b>>x>>y>>z) {
          lights.push_back(std::make_shared<PointLight>(Color(r,g,b), Point3D(x,y,z)));
        } else {
          std::cerr << "Error." << std::endl;
        }
      }else if(command == "spot_light:"){
        float r,g,b,px,py,pz,dx,dy,dz,a1,a2;
        if (ss >> r>>g>>b>>px>>py>>pz>>dx>>dy>>dz>>a1>>a2) {
          lights.push_back(std::make_shared<SpotLight>(Color(r,g,b), Point3D(px,py,pz), Dir3D(dx,dy,dz), a1, a2));
        } else {
          std::cerr << "Error." << std::endl;
        }
      }else if(command == "ambient_light:"){
        float r,g,b;
        if (ss >> r>>g>>b) {
          ambientLight = Color(r,g,b);
        } else {
          std::cerr << "Error." << std::endl;
        }
      }else if(command == "max_depth:"){
        float d;
        if (ss >> d) {
          maxDepth = d;
        } else {
          std::cerr << "Error." << std::endl;
        } 
      }else if (command == "max_vertices:") {
        int n;
        if (ss >> n) { 
            vertices.reserve(n);
        } else {
            std::cerr << "Error: 'max_vertices' command requires 1 numeric parameter." << std::endl;
        }
      }else if (command == "max_normals:") {
        int n;
        if (ss >> n) { 
            normals.reserve(n);
        } else {
            std::cerr << "Error: 'max_normals' command requires 1 numeric parameter." << std::endl;
        }
      }else if (command == "vertex:") {
        if (vertices.capacity() == 0) {
          std::cerr << "Error: 'vertex' specified before 'max_vertices'!\n";
          std::exit(EXIT_FAILURE);
        }
        float x, y, z;
        if (ss >> x >> y >> z) { 
          vertices.emplace_back(x,y,z);
        } else {
            std::cerr << "Error: 'vertex' command requires 3 numeric parameters." << std::endl;
        }
      }else if (command == "normal:") {
        if (normals.capacity() == 0) {
          std::cerr << "Error: 'normal' specified before 'max_normals'!\n";
          std::exit(EXIT_FAILURE);
        }
        float x, y, z;
        if (ss >> x >> y >> z) { 
          normals.emplace_back(x,y,z);
        } else {
            std::cerr << "Error: 'normal' command requires 3 numeric parameters." << std::endl;
        }
      }else if (command == "triangle:") {
        int v1, v2, v3;
        if (ss >> v1 >> v2 >> v3) { 
            auto newTri = std::make_shared<Triangle>(
            v1,v2,v3, ambientColor, diffuseColor, specularColor, 
            phongCos, transmissiveColor, indexORefract);
            sceneObjects.push_back(newTri);
        } else {
            std::cerr << "Error: 'triangle' command requires 3 numeric parameters." << std::endl;
        }
      }else if (command == "normal_triangle:") {
        int v1, v2, v3, n1, n2, n3;
        if (ss >> v1 >> v2 >> v3 >> n1 >> n2 >> n3) { 
            auto newTri = std::make_shared<Triangle>(
            v1,v2,v3, n1,n2,n3, ambientColor, diffuseColor, specularColor, 
            phongCos, transmissiveColor, indexORefract);
            sceneObjects.push_back(newTri);
        } else {
            std::cerr << "Error: 'normal_triangle' command requires 6 numeric parameters." << std::endl;
        }
      }
    }
  }

  // Create an orthogonal camera basis, based on the provided up and forward vectors
  //cast to multivectors
  MultiVector forwardMV = MultiVector(forward.x,forward.y,forward.z);
  MultiVector upMV = MultiVector(up.x,up.y,up.z);
  if ((forwardMV.dot(upMV)).s!=0){//if forward and up not orthogonal, fix it by projecting up onto the plane perpendicular to forward (subtract parallel component of up from up)
    upMV = (upMV - (upMV.dot(forwardMV))*forwardMV).normalized();
    up= Dir3D(upMV);
  }
  //compute cross product
  float x1,y1,z1,x2,y2,z2;
  x1 = up.x;
  y1 = up.y;
  z1 = up.z;
  x2 = forward.x;
  y2 = forward.y;
  z2 = forward.z;
  float rx,ry,rz;
  rx = (y1 * z2) - (z1 * y2);
  ry = (z1 * x2) - (x1 * z2);
  rz = (x1 * y2) - (y1 * x2);
  right = Dir3D(rx,ry,rz);

  printf("Orthogonal Camera Basis:\n");
  forward.print("forward");
  right.print("right");
  up.print("up");
}

#endif