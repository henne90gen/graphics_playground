#include "RotationCommandStack.h"

std::string to_string(RotationCommandStack &cmdStack) {
    std::string result;
    for (RotationCommand &cmd : cmdStack.getAllCommands()) {
        result += to_string(cmd, true) + " -> ";
    }
    return result + "END";
}
