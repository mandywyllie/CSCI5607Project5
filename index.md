
# SDF Ray Tracer

## Animations
Each animation executable first uses the raytracer to render all frames, then prints the relevant ffmpeg command to stitch them together into a video.

### SDF Union Examples
<video src="outdoor_new4.mp4" controls style="width: 90%;"></video>
Created with `outdoor_animate.exe`.

### SDF Morph Example
<video src="torus_morph_old.mp4" controls style="width: 90%;"></video>
Created with `torus_morph.exe`.

## Scenefile commands and usage
The raytracer works by reading a plaintext scenefile. Instructions for creating the the scenefile are <a href="https://github.com/mandywyllie/CSCI5607Project5/blob/main/SceneFile.pdf">here</a>. The SDF ray tracer supports all those original commands, and implements these new commands:

| Function | Parameters | Description |
| :--- | :--- | :--- |
| `sdf_sphere` | `x y z r` | `(x,y,z)` is the position of the sphere's center, `r` is the radius of the sphere. |
| `sdf_box` | `x, y, z, hx, hy, hz, dx, dy, dz` | `(x,y,z)` is the position of the box's center, `hx,hy, hz` are the half lengths of the box's sides, and `(dx,dy,dz)` is the direction the top of the box will face. |
| `sdf_torus` | `x, y, z, r1, r2, dx, dy, dz` | `(x,y,z)` is the position of the torus' center, `r1` is the radius controling the size of the ring, `r2` is the radius controling the thickness of the ring, and `(dx,dy,dz)` is the direction the top of the torus will face. |
| `sdf_union` | `k` | Creates a smooth union between objects A and B with a blend factor of `k`. Requires 2 sdf objects to be created before the union command. The default is `k=0.3`. `k=0` results in no smoothing between objects, i.e. a hard union, while higher values of k creates more blending between objects.|
| `sdf_morph` | `t` | Creates an "in-between" object at time `t` in morphing from object A to object B. Requires 2 sdf objects to be created before the morph command. Displays object A at `t=0`, and object B at `t=1`. `t` should optimally be between 0 and 1, however other values will still render and creates some fun images! |

[//]: # (TODO add example usage here)


## Compiling and running the raytracer
  
  1. To Compile
    With Checks for memory issues: 
        `g++ -fsanitize=address -std=c++11 rayTracer.cpp`
    Without Checks for memory issues:
        `g++ -std=c++11 rayTracer.cpp`

1. to take in a scene file and output an image:
   `.\ray.exe *path to scenefile* `   
   Ex: `.\ray.exe .\ExampleScenes\sdf_box.txt `



## Project Files
You can view the files on github <a href="https://github.com/mandywyllie/CSCI5607Project5/">here</a>, or download the project zip <a href="https://github.com/mandywyllie/CSCI5607Project5/blob/main/CSCI5607project5.zip">here</a>.

