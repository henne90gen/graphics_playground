#pragma once

#include <vector>

#include "RubiksCubeData.h"

class RotationCommandStack {
public:
    void clear() {
        commands.clear();
        nextCommandIndex = 0;
        beenCleared = true;
    }

    void push(RotationCommand cmd) {
        commands.push_back(cmd);
    }

    RotationCommand next() {
        RotationCommand command = commands[nextCommandIndex];
        nextCommandIndex++;
        beenCleared = false;
        return command;
    }

    bool hasNext() {
        return !commands.empty() && nextCommandIndex < commands.size();
    }

    RotationCommand reset() {
        nextCommandIndex = 0;
        return next();
    }

    bool hasCommands() {
        return !commands.empty();
    }

    std::vector<RotationCommand> getAllCommands() {
        return commands;
    }

    bool hasBeenCleared() {
        return beenCleared;
    }

private:
    std::vector<RotationCommand> commands;
    unsigned int nextCommandIndex = 0;
    bool beenCleared = false;
};


std::string to_string(RotationCommandStack &cmdStack);
