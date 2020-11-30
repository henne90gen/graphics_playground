#include "RubiksCubeData.h"

std::string to_string(Face &face, bool simple) {
    switch (face) {
        case FRONT:
            if (simple) {
                return "R_F";
            } else {
                return "FRONT";
            }
        case BACK:
            if (simple) {
                return "R_BA";
            } else {
                return "BACK";
            }
        case LEFT:
            if (simple) {
                return "R_L";
            } else {
                return "LEFT";
            }
        case RIGHT:
            if (simple) {
                return "R_R";
            } else {
                return "RIGHT";
            }
        case TOP:
            if (simple) {
                return "R_T";
            } else {
                return "TOP";
            }
        case BOTTOM:
            if (simple) {
                return "R_BO";
            } else {
                return "BOTTOM";
            }
    }
}

std::string to_string(Direction &dir, bool simple) {
    if (dir == CLOCKWISE) {
        if (simple) {
            return "";
        }
        return "CLOCKWISE";
    }
    if (simple) {
        return "I";
    }
    return "COUNTER_CLOCKWISE";

}

std::string to_string(RotationCommand &cmd, bool simple) {
    std::string separator;
    if (simple) {
        separator = "";
    } else {
        separator = ", ";
    }
    return to_string(cmd.face, simple) + separator + to_string(cmd.direction, simple);
}
