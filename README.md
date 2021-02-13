# rtrt
Library for real-time ray tracing on the CPU

# Description
The library API is loosely inspired by OpenGL: C-style functions that encapsulate a more complex C++ implementation.

Main headers are located in include/rt directory:
* rt.h: Core Programming Interface - main functions for scene setup and raytracing control
* rts.h: Shader Programming Interface - helper functions to implement customized shading effects (after a hit has been found)

Interface classes define customization points in include/rts directory:
* ICamera.h: generate primary rays given a screen coordinate
* IEnvironment.h: compute color for rays that do not hit anything in the scene
* ILight.h: compute radiance at a hit location
* IMaterial.h: compute surface color at a hit location
* IPlugin.h: base class for all interfaces
* IRenderer.h: control generation of primary rays and rendering of a single frame
* ITexture.h: change material shading according to texture image
* RTState.h: encapsulate state information to be accessed by plugins

Several ready-made customization classes are already implemented in include/rtl directory:
* AdaptiveRenderer.h: generates primary rays using adaptive supersampling for antialiasing 
* DepthMaterial.h: computes shading according to distance from camera
* Headlight.h: light fixed at camera position
* HeadlightColor.h: light fixed at camera position with custom color
* JitteredRenderer.h: generates primary rays using supersampling with a jittered grid for antialiasing
* MultiThreadRenderer.h: each primary ray is traced in a different thread
* PackedTiledRenderer.h: traces primary rays in packets using tiles
* PerspectiveCamera.h: traditional 3D camera
* PhongColorMaterial.h: material with phong illumination and a self color
* PhongMaterial.h: material with phong illumination
* SimpleAreaLight.h: light with an associated geometry for soft shadows
* SimplePointLight.h: light concentrated in a single point for hard shadows
* SingleColorEnvironment.h: fixed background color
* SingleRenderer.h: traditional tracing of a single ray per image pixel
* Texture2D.h: support for surface textures
* TiledRenderer.h: generates primary rays using tiles for spatial coherency

Linear algebra and utility classes are implemented in include/rtu and include/rtut.

# Implementation Details
The main C++ implementation is located in src/rtc directory.

The library implements a two-level kd-tree spatial acceleration structure. The first level handles geometry instances and built faster using a simple heuristic towards fast rebuild times. The second level handles each individual geometry and is build slower using a complex heuristic towards improve ray traversal performance.

The kd-tree is constructed top to bottom using a greedy algorithm that tries to optimize a cost function that evaluates whether it is worthwhile to subdivide the current node into two children. This algorithm is known as the Surface Area Heuristic: [On building fast kd-Trees for Ray Tracing, and on doing that in O(N log N)](http://www.irisa.fr/prive/kadi/Sujets_CTR/kadi/Kadi_sujet2_article_Kdtree.pdf)

# Results

Here are some examples images generated:

![boat](https://github.com/potato3d/rtrt/imgs/boat.png "Boat with shadows")
![platform](https://github.com/potato3d/rtrt/imgs/oil_platform.png "Oil platform with textures and shadows")
![reflection](https://github.com/potato3d/rtrt/imgs/reflection.png "Sphere with reflection")
![soft4](https://github.com/potato3d/rtrt/imgs/soft_shadow_4_samples.png "Soft shadows with 4 samples per pixel (jittered)")
![soft16](https://github.com/potato3d/rtrt/imgs/soft_shadow_16_samples.png "Soft shadows with 16 samples per pixel (jittered)")

Performance was ok for the time (2006), considering many optimizations were missing (e.g. SIMD):

![speed table](https://github.com/potato3d/rtrt/imgs/speed.png "Performance results")

# Build
Visual Studio projects available at visualstudio directory.