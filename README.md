# Epitech-CG-Project

This is a project i made for the Epitech Modern 3D rendering with OpenGL course, this project is made with C++ using OpenGL 4.5.

The goal was to render a desert mesh along 1541 palms with OpenGL, and adding lighting to it, i went beyond by adding additionnal features.

## Features

* Rendering of desert mesh
* Rendering of palms using instanced drawing
* Procedural texturing of desert using voronoise
* Coloring of palms in shader
* Directional lighting with specular lighting using phong illumination model
* Shadows with shadow mapping
* Frustum culling of palms for optimization using a compute shader
* HDR and gamma correction
* Bloom effect

## What it looks like

<p align="center">
    <img src='https://cdn.discordapp.com/attachments/1063126184331853935/1071526571271270410/image.png' alt='Render example'>
</p>

## How To Run

You can download the executable file in the releases [Here](https://github.com/LightFox7/Epitech-CG-Project/releases)

If you want to edit the project on your own, you can follow these steps (Requires CMake and Visual Studio)

* Clone the project
* Download the release and copy the .obj files to the "res" folder (too heavy for git)
* Run CMake Generate
* Open the generated solution in the "build" folder
* Run the project in visual studio

That's it !