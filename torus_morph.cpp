//To Compile: g++ -std=c++11 -O3 -fopenmp .\torus_morph.cpp -o torus_morph

//Images Lib includes:
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files

//external packages to include
#include <iomanip>

//header files to include
#include "ray_trace.h"

int main(int argc, char** argv) {
    int totalFrames = 120;
    float fps = 30.0f;

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
        sceneFile << R"(#Simple sdf_sphere: Scene
camera_pos: -6 2 -4
camera_fwd: -.77 0 -.64
camera_up:  0 1 0
camera_fov_ha: 35
output_image: sdf_box2.png

#"ground" sphere:
material: .75 .75 .75 .75 .75 .75 .3 .3 .3 32 .2 .2 .2 1.5
sphere: 0 -50 0 50

#red sphere:
material: 1 0 0 1 0 0 .3 .3 .3 32 .2 .2 .2 1.5
#sdf_box: -3 1 0 .75 .75 .75 1 1 0
sdf_torus: -2 3 6 1 .3 0 0 1


#green sphere:
material: 0 .7 0 0 .7 0 0 0 0 16 .9 .9 .9 1.1
sdf_torus: -2 1 4 1 .3 0 0 1
sdf_morph: )" << t << R"(
sdf_box: 0 2 -2 1 1 1 1 1 0


#blue sphere:
material: 0 0 1 0 0 1 0 0 0 16 0 0 0 1.0
sdf_sphere: 1 1.5 0 1.25

sdf_sphere: 5 1 1 0.25

#sdf_union

#white overhead light
point_light: 10 10 10 0 5 0
ambient_light: .25 .25 .25
background: .05 .05 .05

max_depth: 5)"; 
                            
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