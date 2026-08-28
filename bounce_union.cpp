//To Compile: g++ -std=c++11 -O3 -fopenmp .\bounce_union.cpp -o bounce_union

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

        float sphereX = -1.0f + 0.5f * std::sin(time * 3.0f); 
        float blendK = 0.5f + 0.4f * std::cos(time * 2.0f);  

        //write temporary scenefile using the value of the variables at this time
        sceneFile << "# Frame " << frame << " scene definition\n";
        sceneFile << "camera_pos: -6 2 -4\n"
                    "camera_fwd: -.77 0 -.64\n"
                    "camera_up:  0 1 0 \n"
                    "camera_fov_ha: 35\n"
                    "output_image: sdf_box.png\n"

                    "#ground sphere: \n"
                    "material: .75 .75 .75 .75 .75 .75 .3 .3 .3 32 .2 .2 .2 1.5 \n"
                    "sdf_sphere: 0 -50 0 50 \n"

                    "#red sphere: \n"
                    "material: 1 0 0 1 0 0 .3 .3 .3 32 .2 .2 .2 1.5\n"
                    "#sdf_sphere: -3 1 0 .75\n"
                    "sdf_sphere: 3 1.25 0 .75\n"

                    "#green sphere:\n"
                    "material: 0 .7 0 0 .7 0 0 0 0 16 .9 .9 .9 1.1\n"
                    "sdf_box: 0 1.25 0 .3 2 .3\n"

                    "#blue sphere:\n"
                    "material: 0 0 1 0 0 1 0 0 0 16 0 0 0 1.0\n"
                    "sdf_sphere: "<< sphereX <<" 1.5 .7 .5\n"

                    "sdf_union:\n"

                    "#white overhead light\n"
                    "point_light: 10 10 10 0 5 0\n"
                    "ambient_light: .25 .25 .25\n"
                    "background: .05 .05 .05\n"

                    "max_depth: 5\n"; 
                            
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
    std::cout << "ffmpeg -framerate 30 -i output_frame_%03d.png -c:v libx264 -pix_fmt yuv420p bounce_union.mp4\n";
    std::cout << "Remove pngs using:\n";
    std::cout << "Remove-Item output_frame_*.png\n";

    return 0;
}