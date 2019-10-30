# Graphics Playground

In this project I'm trying out different graphics algorithms and visualizations.
Each scene has a small panel with settings which affect the algorithm in some way.
This way one can see the effect of different parameters in real time.

## Setup

1. Checkout the git repository recursively with `git clone --recursive ...`
2. Build the executable
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```
3. Go into the directory `build/src/app` and run the `Playground` executable

## Usage

Each Scene has a settings menu and a navigation menu.
To get back to the main menu, just press the back button in the navigation menu.

## Scenes

### Test Scene

Empty scene that demonstrates how to setup a scene.

### Legacy Triangle

Uses the legacy OpenGL fixed function pipeline to draw a simple triangle.

### Triangle

Uses the modern OpenGL shader pipeline to draw a simple triangle.

### Texture Demo

Demonstrates how to map a texture to a quad.

### Gamma Calculation

The gamma value of a display is the specific exponent of the displays color translation function.

![Gamma Calculation](screenshots/gamma-calculation.gif)

### Cube

Shows a simple 3D cube.

### Landscape

In this scene a landscape is being generated from different noise algorithms.
The starting point is a completely flat plane.
Each vertex of that plane has a 2D position on the plane (ignoring the height for now).
We can take this 2D position and use it as an input for one of our noise functions.
Depending on the noise function that is selected, the resulting value can be more or less similar to values of neighboring positions.
Now we can use the value we have calculated for each position as the height at that position.

![Landscape](screenshots/landscape.gif)

### Rubiks Cube

Here you can see a rubiks cube.
It can be shuffled by pressing a button.
Maybe I will get around to implementing a solver at some point.

### Marching Cubes

Generating interesting terrains is a big part of most procedural games.
The marching cubes algorithm can be used to do just that.
The basis of this algorithm is a cube, from which we select corners.
Depending on which corners we selected, we add the corresponding vertices and thus triangles to the mesh.
A cube has 8 corners, thus we have 2^8 possible combinations of these corners.
From all these combinations we pre-generate a table that contains the corresponding vertices as explained above.
The next step is to place the small cube into the big volume.
This is where the marching comes into play.
The small cube moves through the big volume and at each position we decide for every corner of the cube whether it is inside of the terrain or outside of it.
To decide that, we use a function that accepts 3D coordinates and gives us a value between 0.0 and 1.0 back.
The surface of our terrain is then defined by a threshold between 0.0 and 1.0.
A corner of the small cube is inside the terrain if the value of the function is below the threshold.
For each position of the cube we use the result of checking each corner whether its inside the terrain or not to query the pre-generated table for the corresponding vertices.
If we do this for every possible position inside of the volume, then we'll get the surface of the terrain.
To make the surface less "blocky" we can use the result of the function to interpolate the vertex positions between the corners.
The end result looks something like this:

![Marching Cubes](screenshots/marching-cubes.gif)

### Font Demo

This demo shows how to render fonts using freetype2.
The selected font is loaded into memory.
After that, each character is rendered into a bitmap.
These bitmaps are then used as texture for a quad.
To ensure correct placement, we have to save additional information, such as kerning, with each character.

![Font Demo](screenshots/font-demo.png)

### Model Loading

This is a simple `*.obj` file loader.
Just place any model that you want to load into `src/app/scenes/model_loading/models` and select it in the settings menu.
The loader is very simplistic and can only handle very simple models correctly.

### Light Demo

Shows what the Phong reflection model looks like.
The different components of the model can be switched on and off individually.
It is also possible to adjust the color of the ambient and specular components, as well as the light color.

![Light Demo](screenshots/light-demo.gif)

### Fourier Transform

### Normal Mapping

Normal mapping or bump mapping is a technique to increase the visual fidelity of surfaces without increasing the complexity of the underlying mesh.
The idea is fairly simple. Instead of one normal per surface, we create one normal per fragment.
We can do this by pre-calculating them and storing them in a special texture called normal map.
The only thing we need to consider when applying the normal map, is the fact that the surface we are applying it to might not have the same orientation in space as the surface we created the normal map for.
This can be easily corrected by calculating the normal, tangent and bi-tangent vectors for the surface.
The end result can be seen below.

![Normal Mapping](screenshots/normal-mapping.gif)

### A*

A* is a search algorithm that finds the shortest path from one point to another in a graph.
It uses a heuristic to improve the searching behavior.
We start out doing a breadth first search.
The only difference is that the queue that we are using is a priority queue.
The nodes in the priority queue are sorted by their total estimated distance to the target.
We get this value by keeping track of how far we have traveled so far and then adding whatever the heuristic says how far we are still away from the target.

### 2D Shadows

This demo shows how to cast shadows in a 2D environment.
The basic algorithm consists of 3 main parts.

#### 1. Ray casting

At first we cast rays from the light source into the scene.
We cast as many rays as we have corners of obstacles in the scene.

#### 2. Calculate closest intersections

Next we calculate the closest intersection point with any obstacle for each ray.

#### 3. Calculate light/shadow mesh

With this information we can go to the final stage of the process, calculating a mesh that covers the light area.
To cover the light area, we sort all the closest intersection points in a circle.
This ensures that two neighboring points in the array are also next to each other on an imaginary circle around the light source.
With the points sorted in this manner, we can simply draw a triangle fan around the light source.
This will then cover the whole lit area.

We can then use the stencil buffer to create the illusion of shadows.
This can be achieved by setting up the stencil in such a way that the lit area is the only area that passes the stencil test.
Thus the lit area is the only area that is drawn to. The rest of the scene lies in darkness.

### Ray Casting 3D

### Water Demo

## Additional Features

### Screenshots

The back menu offers the option to take screenshots of the current scene.
The screenshot is then saved to the projects working directory as `screenshot-%year%-%month%-%day%-%hour%:%minute%:%second%.png`.

### Screen Recording

The back menu offers the option to take screen recordings of the current scene.
Choose one of GIF or MP4 and press the `Start Recording` button.
After you're done recording, just press the `Stop Recording` button.
The recording is then stored in the projects working directory as `screenshot-%year%-%month%-%day%-%hour%:%minute%:%second%.%extension%`
The extension is determined by the recording type you chose.

### Timing Utility
