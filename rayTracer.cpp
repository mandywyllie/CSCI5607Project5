//To Compile: g++ -fsanitize=address -std=c++11 rayTracer.cpp

//Images Lib includes:
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files

#include "ray_trace.h"


int main(int argc, char** argv){
  
  //Read command line parameters to get scene file
  if (argc != 2){
     std::cout << "Usage: ./a.out scenefile\n";
     return(0);
  }
  std::string sceneFileName = argv[1];

  //Parse Scene File
  parseSceneFile(sceneFileName);

  auto t_start = std::chrono::high_resolution_clock::now();
  Image outputImg = RayCast(img_width,img_height); //set in parse header file
  auto t_end = std::chrono::high_resolution_clock::now();
  printf("Rendering took %.2f ms\n",std::chrono::duration<double, std::milli>(t_end-t_start).count());

  outputImg.write(imgName.c_str());
  return 0;
}
