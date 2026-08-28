
# SDF Ray Tracer
1. To Compile
    With Checks for memory issues: 
        `g++ -fsanitize=address -std=c++11 rayTracer.cpp`
    Without Checks for memory issues:
        `g++ -std=c++11 rayTracer.cpp`

2. to take in a text file and output an image:
   `.\ray.exe *path to scenefile* `   
   Ex: `.\ray.exe .\ExampleScenes\sdf_box.txt `


# Animation creation executables
Each animation creation executable first renders all frames, then prints the relevant ffmpeg command to stitch them together into a video.

## SDF Union Examples
<video src="outdoor_new4.mp4" controls style="width: 80%;"></video>


## SDF Morph Example
<video src="torus_morph_old.mp4" controls></video>


# Project Zip
<a href="https://github.com/mandywyllie/CSCI5607Project2/blob/main/project2Graphics.zip">Project Zip</a>

