#version 300 es
precision mediump float;

// much of the code was taken from: https://www.shadertoy.com/view/3sKSWc

// goes from (0,0) to (1,1), bottom-left to top-right
in vec2 positionScreenSpace;

uniform vec2 resolution;
uniform vec2 horizontalCoordinateBounds;
uniform vec2 verticalCoordinateBounds;
uniform float axisWidth;
uniform float lineWidth;

out vec4 color;

float my_function(float x) {
    float a = x - 5.0;
    return a * a - 1.0;
}

float InvLerp(float a, float b, float v) {
    return (v - a) / (b - a);
}

float distanceToLineSegment(vec2 p0, vec2 p1, vec2 p) {
    float distanceP0 = length(p0 - p);
    float distanceP1 = length(p1 - p);

    float l2 = pow(length(p0 - p1), 2.);
    float t = max(0., min(1., dot(p - p0, p1 - p0) / l2));
    vec2 projection = p0 + t * (p1 - p0);
    float distanceToProjection = length(projection - p);

    return min(min(distanceP0, distanceP1), distanceToProjection);
}

float distanceToFunction(vec2 p, float xDelta) {
    float result = 100.0;

    for (float i = -3.0; i <= 3.0; i += 1.0) {
        vec2 q = p;
        q.x += xDelta * i;

        vec2 p0 = vec2(q.x, my_function(q.x));
        vec2 p1 = vec2(q.x + xDelta, my_function(q.x + xDelta));
        result = min(result, distanceToLineSegment(p0, p1, p));
    }

    return result;
}

vec4 axis(vec4 color, float perpendicularScale, float coordinate) {
    float t = abs(step(perpendicularScale * axisWidth, abs(coordinate)) - 1.0F);
    color = mix(color, vec4(0.0F), t);
    return color;
}

vec2 calcPositionCoordinateSpace(vec2 positionScreenSpace, vec2 horizontalCoordinateBounds, vec2 verticalCoordinateBounds, vec2 resolution) {
    vec2 positionCoordinateSpace = vec2(
            mix(horizontalCoordinateBounds.x, horizontalCoordinateBounds.y, positionScreenSpace.x),
            mix(verticalCoordinateBounds.x, verticalCoordinateBounds.y, positionScreenSpace.y)
        );
    float aspectRatio = resolution.x / resolution.y;
    if (aspectRatio > 1.0) {
        positionCoordinateSpace = vec2(positionCoordinateSpace.x * aspectRatio, positionCoordinateSpace.y);
    } else {
        positionCoordinateSpace = vec2(positionCoordinateSpace.x, positionCoordinateSpace.y / aspectRatio);
    }
    return positionCoordinateSpace;
}

void main() {
    float epsilon = 0.00001F;

    vec2 positionCoordinateSpace = calcPositionCoordinateSpace(positionScreenSpace, horizontalCoordinateBounds, verticalCoordinateBounds, resolution);
    float horizontalSize = horizontalCoordinateBounds.y - horizontalCoordinateBounds.x;
    float verticalSize = verticalCoordinateBounds.y - verticalCoordinateBounds.x;

    color = vec4(1.0F);

    // draw function
    float distanceToPlot = distanceToFunction(positionCoordinateSpace, (horizontalSize / resolution.x));
    distanceToPlot *= (resolution.x + resolution.y) / 2.0;
    distanceToPlot /= (horizontalSize + verticalSize);
    distanceToPlot /= lineWidth * lineWidth;
    float t = 1.0 - distanceToPlot;
    float intensity = smoothstep(0.0, 1.0, t);
    intensity = pow(intensity, 1.0 / 2.2);
    color = vec4(vec3(1.0) - vec3(intensity), 1.0);

    // draw x axis
    color = axis(color, verticalSize, positionCoordinateSpace.y);

    // draw y axis
    color = axis(color, horizontalSize, positionCoordinateSpace.x);
}
