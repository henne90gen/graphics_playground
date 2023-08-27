#pragma once

#include <array>
#include <string>
#include <vector>

namespace rubiks {

/*
Global Indices:
           UP
            6  7  8
           15 16 17
           24 25 26
          ----------
LEFT     | FRONT    | RIGHT    | BACK
 6 15 24 | 24 25 26 | 26 17  8 | 8  7  6
 3 12 21 | 21 22 23 | 23 14  5 | 5  4  3
 0  9 18 | 18 19 20 | 20 11  2 | 2  1  0
          ----------
           DOWN
           18 19 20
            9 10 11
            0  1  2

Local Indices:
       UP
       0 1 2
       3 4 5
       6 7 8
LEFT   FRONT  RIGHT  BACK
0 1 2  0 1 2  0 1 2  0 1 2
3 4 5  3 4 5  3 4 5  3 4 5
6 7 8  6 7 8  6 7 8  6 7 8
       DOWN
       0 1 2
       3 4 5
       6 7 8
*/

#define FRONT_CUBES                                                                                                    \
    { 24, 25, 26, 21, 22, 23, 18, 19, 20 }
#define BACK_CUBES                                                                                                     \
    { 8, 7, 6, 5, 4, 3, 2, 1, 0 }
#define RIGHT_CUBES                                                                                                    \
    { 26, 17, 8, 23, 14, 5, 20, 11, 2 }
#define LEFT_CUBES                                                                                                     \
    { 6, 15, 24, 3, 12, 21, 0, 9, 18 }
#define UP_CUBES                                                                                                       \
    { 6, 7, 8, 15, 16, 17, 24, 25, 26 }
#define DOWN_CUBES                                                                                                     \
    { 18, 19, 20, 9, 10, 11, 0, 1, 2 }

// macros for cube rotations (I = inverse)
#define R_R                                                                                                            \
    { rubiks::Face::RIGHT, rubiks::Direction::CLOCKWISE }
#define R_RI                                                                                                           \
    { rubiks::Face::RIGHT, rubiks::Direction::COUNTER_CLOCKWISE }
#define R_F                                                                                                            \
    { rubiks::Face::FRONT, rubiks::Direction::CLOCKWISE }
#define R_FI                                                                                                           \
    { rubiks::Face::FRONT, rubiks::Direction::COUNTER_CLOCKWISE }
#define R_D                                                                                                            \
    { rubiks::Face::DOWN, rubiks::Direction::CLOCKWISE }
#define R_DI                                                                                                           \
    { rubiks::Face::DOWN, rubiks::Direction::COUNTER_CLOCKWISE }
#define R_L                                                                                                            \
    { rubiks::Face::LEFT, rubiks::Direction::CLOCKWISE }
#define R_LI                                                                                                           \
    { rubiks::Face::LEFT, rubiks::Direction::COUNTER_CLOCKWISE }
#define R_U                                                                                                            \
    { rubiks::Face::UP, rubiks::Direction::CLOCKWISE }
#define R_UI                                                                                                           \
    { rubiks::Face::UP, rubiks::Direction::COUNTER_CLOCKWISE }
#define R_B                                                                                                            \
    { rubiks::Face::BACK, rubiks::Direction::CLOCKWISE }
#define R_BI                                                                                                           \
    { rubiks::Face::BACK, rubiks::Direction::COUNTER_CLOCKWISE }

// NOTE Do not change the numeric values of these enum constants. They are being used to index into arrays.
enum class Face {
    NONE = 0,
    FRONT = 1,
    BACK = 2,
    LEFT = 3,
    RIGHT = 4,
    UP = 5,
    DOWN = 6,
};

enum class Direction { CLOCKWISE, COUNTER_CLOCKWISE };

struct RotationCommand {
    Face side = {};
    Direction direction = {};
};

const unsigned int CUBELET_COUNT = 27;
const unsigned int SIDE_COUNT = 6;
const unsigned int SMALL_FACE_COUNT = 9;
extern const std::array<std::array<unsigned int, SMALL_FACE_COUNT>, SIDE_COUNT> WHOLE_CUBE;

struct CoreRubiksCube {
    std::array<unsigned int, CUBELET_COUNT> globalIndexToCubeletIndex;
    std::array<std::array<Face, SMALL_FACE_COUNT>, SIDE_COUNT> sideAndLocalIndexToFace;

    CoreRubiksCube();
    CoreRubiksCube(const std::vector<RotationCommand> &commands);

    /**
     * Applies the given RotationCommand to this Rubiks cube.
     */
    void rotate(RotationCommand cmd);

    /**
     * Applies the given sequence of RotationCommands to this Rubiks cube in the order they are given in.
     */
    void rotate(const std::vector<RotationCommand> &commands);

    /**
     * @brief Get the current face at the provided local index.
     *
     * This method looks at the given face of the cube and returns to which face the piece at the given local index
     * belongs to.
     *
     * @param side Side to look at
     * @param localIndex Local index to look at
     * @return Face that is present at the selected position
     */
    Face getCurrentFace(Face side, unsigned int localIndex);
    Face getCurrentFace(const std::pair<Face, unsigned int> &pair);

    /**
     * Solves this Rubiks cube and returns the list of RotationCommands that were applied.
     * The cube is in a solved configuration after this method has been called, the same as calling the constructor
     * CoreRubiksCube().
     */
    std::vector<RotationCommand> solve();

    CoreRubiksCube copy();

  private:
    void init();

    void adjustCubeletIndicesClockwise(std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes);
    void adjustCubeletIndicesCounterClockwise(std::array<unsigned int, SMALL_FACE_COUNT> &selectedCubes);
    void adjustFaceIndicesClockwise(Face side);
    void adjustFaceIndicesCounterClockwise(Face side);

    void solveBottomLayer(std::vector<RotationCommand> &result);
    void solveCreateBottomCross(std::vector<RotationCommand> &result);
    void solveBottomCornerPieces(std::vector<RotationCommand> &result);
    void solveMiddleLayer(std::vector<RotationCommand> &result);
    void solveTopLayer(std::vector<RotationCommand> &result);
    void solveCreateTopCross(std::vector<RotationCommand> &result);
};

} // namespace rubiks

std::string to_string(const rubiks::Face &face, bool simple = false);

std::string to_string(const rubiks::Direction &dir, bool simple = false);

std::string to_string(const rubiks::RotationCommand &cmd, bool simple = false);
