#version 300 es
precision mediump float;

in vec2 coords;

uniform vec2 boundsX;
uniform vec2 boundsY;
uniform float axisWidth;

out vec4 color;

float my_function(float x) {
    return x*x;
}

float InvLerp(float a, float b, float v) {
    return (v - a) / (b - a);
}

void main() {
    float epsilon = 0.0001F;

    float leftXScreen = coords.x - epsilon;
    float leftX = mix(boundsX.x, boundsX.y, leftXScreen);
    float leftY = my_function(leftX);

    float rightXScreen = coords.x + epsilon;
    float rightX = mix(boundsX.x, boundsX.y, rightXScreen);
    float rightY = my_function(rightX);

    float expectedX = mix(boundsX.x, boundsX.y, coords.x);
    float expectedY = mix(boundsY.x, boundsY.y, coords.y);
    float top = abs((rightX-leftX)*(rightY-expectedY)-(leftX-expectedX)*(rightY-leftY));
    float bottom = sqrt((rightX-leftX)*(rightX-leftX) + (rightY-leftY)*(rightY-leftY));
    float distance = top / bottom;

    // TODO instead of using the distance, find the closest point on the line to the expected point
    //      use the x and y distance to find the correct color
    //      this should make the thickness of the line independent of the scale

    float horizontalSize = (boundsX.y - boundsX.x);
    float verticalSize = (boundsY.y - boundsY.x);
    float t = clamp(InvLerp(horizontalSize * 0.005F, horizontalSize * 0.01F, distance), 0.0F, 1.0F);
    color = mix(vec4(0.0F), vec4(1.0F), t);

    // draw x axis
    t = abs(step(verticalSize * axisWidth, abs(expectedY)) - 1.0F);
    color = mix(color, vec4(0.0F), t);

    // draw y axis
    t = abs(step(horizontalSize * axisWidth, abs(expectedX)) - 1.0F);
    color = mix(color, vec4(0.0F), t);
}
