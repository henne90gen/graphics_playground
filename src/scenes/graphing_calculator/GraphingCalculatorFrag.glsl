#version 300 es
precision mediump float;

// goes from (0,0) to (1,1), bottom-left to top-right
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
    float epsilon = 0.00001F;

    float expectedX = mix(boundsX.x, boundsX.y, coords.x);
    float expectedY = mix(boundsY.x, boundsY.y, coords.y);

    float leftXScreen = coords.x - epsilon;
    float leftX = mix(boundsX.x, boundsX.y, leftXScreen);
    float leftY = my_function(leftX);

    float rightXScreen = coords.x + epsilon;
    float rightX = mix(boundsX.x, boundsX.y, rightXScreen);
    float rightY = my_function(rightX);

    float m_f = (leftX-rightX) / (leftY-rightY);
    float n_f = leftY - m_f*leftX;

    float m_g = -1.0F / m_f;
    float n_g = expectedY - m_g*expectedX;

    float qX = (n_g - n_f) / (m_f - m_g);
    float qY = m_f*qX + n_f;
    vec2 Q = vec2(qX, qY);

    vec2 P = vec2(expectedX, expectedY);
    vec2 dir = Q - P;
    float horizontalSize = boundsX.y - boundsX.x;
    float verticalSize = boundsY.y - boundsY.x;

    vec2 scaledDir = vec2(
    InvLerp(0.0F, horizontalSize, abs(dir.x)),
    InvLerp(0.0F, verticalSize, abs(dir.y))
    );
    float distance = length(scaledDir);

    float t = InvLerp(0.005, 0.01, distance);
    color = mix(vec4(0.0F), vec4(1.0F), t);

    //    color = vec4(scaledDir, 0.0F, 1.0F);
    //    color = vec4(
    //    InvLerp(boundsX.x, boundsX.y, Q.x),
    //    InvLerp(boundsY.x, boundsY.y, Q.y),
    //    0.0,
    //    1.0
    //    );
//        color = vec4(
//        InvLerp(boundsX.x, boundsX.y, dir.x),
//        InvLerp(boundsY.x, boundsY.y, dir.y),
//        0.0,
//        1.0
//        );

    // draw x axis
    //    t = abs(step(verticalSize * axisWidth, abs(expectedY)) - 1.0F);
    //    color = mix(color, vec4(0.0F), t);

    // draw y axis
    //    t = abs(step(horizontalSize * axisWidth, abs(expectedX)) - 1.0F);
    //    color = mix(color, vec4(0.0F), t);
}
