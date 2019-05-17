#pragma once

#include <vector>

#include "RubiksCubeData.h"

class RotationCommandStack {
public:
    void clear() {
        commands.clear();
        nextCommandIndex = 0;
    }

    void push(RotationCommand cmd) {
        commands.push_back(cmd);
    }

    RotationCommand next() {
        RotationCommand command = commands[nextCommandIndex];
        nextCommandIndex++;
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

private:
    std::vector<RotationCommand> commands;
    unsigned int nextCommandIndex = 0;
};
