#include "RotationCommandStack.h"

std::string RotationCommandStack::to_string() const {
    std::string result;
    for (const RotationCommand &cmd : commands) {
        result += ::to_string(cmd, true) + " -> ";
    }
    return result + "END";
}
