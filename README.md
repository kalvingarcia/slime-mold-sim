# slime-mold-sim
A physarum slime mold simulation using modern OpenGL (core 4.6). Built using C/C++. 
I found some challenges in understanding the concept of compute shaders, but with the help of [Victor Gordon](https://www.youtube.com/watch?v=nF4X9BIUzx0)
I was able to understand the concept and apply it to this project. I am hoping to implement the ability to have multiple slime "families" in the simulation at once,
and possibly the ability to have mulitple simulations running in a small "petri dish" within the window. I was also inspired by 
[Alon Emanuel](https://www.youtube.com/watch?v=fOIL7Gmgbr0) to possibly include food.

![slim-mold-sim-1](https://i.imgur.com/EZu6GJT.gifv)

## Inspiration and References
I originally watched the [Sebastian Lague video](https://www.youtube.com/watch?v=X-iSQQgOd1A&t=1s) on the physarum, which he built using C#. 
I was inspired to create a simillar simulation, but instead using C++. I did some research and 
found an article on the process using openFramworks by [Sage Jenson](https://cargocollective.com/sagejenson/physarum). I decided to use the opportunity 
to learn GLFW and GLEW, as well as OpenGl's Shader Language.

## Installing and Running the Project
I made this project in Visual Studio because I found compiling my own binaries for the MinGW compiler too complex of a process and finally opted to using Visual Studio.
I would recommend downloading the project, and then adding the libraries/lib folder into the external libs, and adding the libraries/includes into the external includes.
Otherwise, the project could potentially be linked manually, but I have yet to have any success in doing so.

![slime-mold-sim-2](https://i.imgur.com/dWmCwNO.gifv)

## Project File Breakdown
```
- libraries - contains the dependencies necessary to compile driver.cpp

- shaders - contains the glsl files that house the shaders the project uses

- shader.h - contains classes that house the program ids

- simulation.h - contains a class that holds the simulation loop of the project that calls the shaders

- driver.cpp - includes the simulation.h class header
```

### License
GNU General Public License v3.0
