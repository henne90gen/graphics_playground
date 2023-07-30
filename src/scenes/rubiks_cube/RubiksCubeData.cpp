#include "RubiksCubeData.h"

std::string to_string(const Face &face, bool simple) {
    switch (face) {
        case Face::FRONT:
            if (simple) {
                return "R_F";
            } else {
                return "FRONT";
            }
        case Face::BACK:
            if (simple) {
                return "R_BA";
            } else {
                return "BACK";
            }
        case Face::LEFT:
            if (simple) {
                return "R_L";
            } else {
                return "LEFT";
            }
        case Face::RIGHT:
            if (simple) {
                return "R_R";
            } else {
                return "RIGHT";
            }
        case Face::TOP:
            if (simple) {
                return "R_T";
            } else {
                return "TOP";
            }
        case Face::BOTTOM:
            if (simple) {
                return "R_BO";
            } else {
                return "BOTTOM";
            }
    }
}

std::string to_string(const Direction &dir, bool simple) {
    if (dir == Direction::CLOCKWISE) {
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

std::string to_string(const RotationCommand &cmd, bool simple) {
    std::string separator;
    if (simple) {
        separator = "";
    } else {
        separator = ", ";
    }
    return to_string(cmd.face, simple) + separator + to_string(cmd.direction, simple);
}
