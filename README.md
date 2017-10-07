# cg2017
This is an final assignment submitted for a Computer Graphics class at the University of Adelaide in 2017, involving myself (jkortman) and another student (jthughes). It uses OpenGL 3.3 and C++11.

A video of the program can be found [here](https://www.youtube.com/watch?v=TNU_dsAG1ac).

Features
---
It's mostly a mess of half-finished ideas, but does contain a number of complete and interesting features:
 - Cel shaded objects and terrain
 - Simultaneous directional, point, and spot light sources
 - OBJ file loading
 - Procedurally generated day/night skybox textures
 - Exponential fog used for depth cues (subtle)
 - Yellow tint nearby sun to imitate light scattering
 - Reflective water
 - Dynamic shadow mapping that changes with the sun position
 - Procedurally generated 3D landscape
 - Ambient occlusion (SSAO)

Compile & run instructions
---
To compile and start the program: `make all && ./assignment3_part2`
Has only really been tested on MaxOS Sierra and Ubuntu (not certain of the version).

Exploring the program:
 - The mouse is used to control the camera direction.
 - W, A, S, and D are used to move forward, left, down, and right respectively.
 - SHIFT and SPACE are used to move up and down respectively.
 - ALT is used to increase movement speed.
 - O is used to start a demo flythrough of the landscape.
 - ESC is used to exit the program.

