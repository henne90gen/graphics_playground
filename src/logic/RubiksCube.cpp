#include "RubiksCube.h"

#include <random>
#include <chrono>
#include <iostream>

void RubiksCube::shuffle() {
    rotationCommands.clear();
    RotationCommand rotations[] = {R_R, R_RI, R_F, R_FI, R_BO, R_BOI, R_L, R_LI, R_T, R_TI, R_BA, R_BAI};
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, 11);
    for (unsigned int i = 0; i < 20; i++) {
        unsigned int randomIndex = distribution(generator);
        rotationCommands.push(rotations[randomIndex]);
    }
    std::cout << "Shuffeling cube..." << std::endl;

    isRotating = true;
    loop = false;
}
