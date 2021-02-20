#version 430 core

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D img_output;

uniform vec3 pixelColor = vec3(1.0F, 0.0F, 1.0F);

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    imageStore(img_output, pixel_coords, vec4(pixelColor, 1.0F));
}
