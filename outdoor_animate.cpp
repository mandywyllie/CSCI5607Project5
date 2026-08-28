//To Compile: g++ -std=c++11 -O3 -fopenmp .\outdoor_animate.cpp -o outdoor_animate

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

        float sphereY = 6.0f * std::sin(time * 3.0f); 

        float blendK = 0.5f + 0.4f * std::cos(time * 2.0f);  

        //write temporary scenefile using the value of the variables at this time
        sceneFile << "# Frame " << frame << " scene definition\n";
        sceneFile << R"(camera_pos: 0 0 -60
camera_fwd: 0 0 -1
camera_up:  0 1 0
camera_fov_ha: 15
output_image: outdoor.png

#left sphere
material: 0 0 0 .2 .2 .2 .1 .1 .1 15 .9 .9 .9 1.5
sdf_sphere: -10 )" << sphereY-5.0f << R"( 10 4

#right sphere
material: 0 0 0 0 0 0 .9 .9 .9 15 0 0 0 1
sdf_sphere: 10 4 10 8
sdf_sphere: )" << 2.0*sphereY+2.0f << " " << -2.0f*sphereY +5 << R"( 10 4

sdf_union: 9

max_vertices: 4

vertex: -2000 -10 -2000
vertex:  2000 -10 -2000
vertex:  2000 -10 2000
vertex: -2000 -10 2000

material: 0 0 0 1 1 1 0 0 0 5 0 0 0 1

triangle: 0 1 2
triangle: 0 2 3

background: 0 0 1

directional_light: 1 1 1 -1 -1 1 
#red directional light directly down
directional_light: 1 0 0 0 -1 0
point_light: 0 100 0 0 -9 10)"; 
                            
        sceneFile.close();

        //use scenefile to render frame
        std::vector<std::shared_ptr<Hittable>> sceneObjects;
        parseSceneFile(sceneFileName.str()); // Assuming parseSceneFile populates sceneObjects
        std::cout << "Rendering frame " << frame + 1 << "/" << totalFrames << "..." << std::endl;
        Image outputImg = RayCast(img_width,img_height); //img_width and img_height set in parse header file
        std::ostringstream imageFileName;
        imageFileName << "output_frame_" << std::setfill('0') << std::setw(3) << frame << ".png";
        outputImg.write(imageFileName.str().c_str());

        //delete temporary scene text file
        std::remove(sceneFileName.str().c_str());
    }

    std::cout << "Animation frames complete! Combine with ffmpeg using:\n";
    std::cout << "ffmpeg -framerate 30 -i output_frame_%03d.png -c:v libx264 -pix_fmt yuv420p animation.mp4\n";
    std::cout << "Remove pngs using:\n";
    std::cout << "Remove-Item output_frame_*.png\n";

    return 0;
}