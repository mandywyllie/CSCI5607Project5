//To Compile: g++ -std=c++11 -O3 -fopenmp .\torus_box.cpp -o torus_box

//Images Lib includes:
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files

//external packages to include
#include <iomanip>

//header files to include
#include "ray_trace.h"

int main(int argc, char** argv) {
    int totalFrames = 30.0f;
    float fps = 10.0f;

    std::cout << "Starting animation sequence generation..." << std::endl;

    for (int frame = 0; frame < totalFrames; ++frame) {
        float time = frame / fps; //use frame number and fps to get current time for this frame
        sceneObjects.clear();
        lights.clear();
        vertices.clear();
        normals.clear();
        //give temp scenefile the corresponding name
        std::ostringstream sceneFileName;
        sceneFileName << "temp_scene_" << std::setfill('0') << std::setw(3) << frame << ".txt";

        //create scenefile
        std::ofstream sceneFile(sceneFileName.str());
        if (!sceneFile.is_open()) {
            std::cerr << "Failed to create scene file for frame " << frame << std::endl;
            return 1;
        }

        float t = std::abs(std::sin(time));
        //write temporary scenefile using the value of the variables at this time
        sceneFile << "# Frame " << frame << " scene definition\n";
        sceneFile << R"(camera_pos: 0 15 -30
camera_fwd: 0 .3 -1
camera_up:  0 1 0
camera_fov_ha: 15
output_image: torus_box.png

material: .7 0 .7 .7 0 .7 0 0 .7 16 0 0 0 1.1
sdf_torus: 0 5 10 9 1 0 1 0
material: 0 0 .7 0 0 .7 0 .7 .7 16 0 0 0 1.1
#sdf_torus: 0 5 10 5 3 0 1 0
#morph from torus to torus
#sdf_morph: )" << t*2.0 << R"(

sdf_box: 0 5 10 9 2 9 0 1 0
#sdf_box: 
#morph from box to box
#sdf_morph: )" << t*2.0 << R"(

#morph from morph to morph
sdf_morph: )" << t << R"(

#sdf_morph: .5

max_vertices: 4

vertex: -2000 -10 -2000
vertex:  2000 -10 -2000
vertex:  2000 -10 2000
vertex: -2000 -10 2000

material: 0 0 0 1 1 1 0 0 0 5 0 0 0 1

triangle: 0 1 2
triangle: 0 2 3

background: .7 .7 .7

directional_light: 1 1 1 -1 -1 1 
#white directional light directly down
directional_light: 1 1 1 0 -1 0
point_light: 0 100 0 0 -9 10
max_depth = 3)"; 
                            
        sceneFile.close();

        //use scenefile to render frame
        std::vector<std::shared_ptr<Hittable>> sceneObjects;
        parseSceneFile(sceneFileName.str());
        std::cout << "Rendering frame " << frame + 1 << "/" << totalFrames << "..." << std::endl;
        Image outputImg = RayCast(img_width,img_height); 
        std::ostringstream imageFileName;
        imageFileName << "output_frame_" << std::setfill('0') << std::setw(3) << frame << ".png";
        outputImg.write(imageFileName.str().c_str());

        //delete temporary scene text file
        std::remove(sceneFileName.str().c_str());
    }

    std::cout << "Animation frames complete! Combine with ffmpeg using:\n";
    std::cout << "ffmpeg -framerate 30 -i output_frame_%03d.png -c:v libx264 -pix_fmt yuv420p torus_morph.mp4\n";
    std::cout << "Remove pngs using:\n";
    std::cout << "Remove-Item output_frame_*.png\n";

    return 0;
}