# Graphics Playground

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

### Cube

Shows a simple 3D cube.

### Landscape

In this scene a landscape is being generated from different noise algorithms.
The starting point is a completely flat plane.
Each vertex of that plane has a 2D position on the plane (ignoring the height for now).
We can take this 2D position and use it as an input for one of our noise functions.
Depending on the noise function that is selected, the resulting value can be more or less similar to values of neighboring positions.
Now we can use the value we have calculated for each position as the height at that position.

### Rubiks Cube

### Marching Cubes

### Font Demo

Demonstrates how to render fonts using freetype2.
The selected font is loaded and then we can render bitmap images for each character.
To ensure correct placement, we have to save additional information, such as kerning, with each character.

### Model Loading

This is a simple `*.obj` file loader.
Just place any model that you want to load into `src/app/scenes/model_loading/models` and select it in the settings menu.
The loader is very simplistic and can only handle very simple models correctly.

### Light Demo

Shows what the Phong reflection model looks like.
The different components of the model can be switched on and off individually.
It is also possible to adjust the color of the ambient and specular components, as well as the light color.

### Fourier Transform

### Normal Mapping

### A*
