#include <glm/glm.hpp>
#include <gtest/gtest.h>

#include "AStarSolver.h"
#include "util/TestUtils.h"

Board initBoard(unsigned int width, unsigned int height) {
    auto pixels = std::vector<glm::vec3>(width * height);
    for (auto &field : pixels) {
        field = backgroundColor;
    }
    return {width, height, pixels};
}

void setColor(Board &board, glm::ivec2 pos, glm::vec3 color) { board.pixels[pos.x + pos.y * board.width] = color; }

void positionEquals(Board &board, glm::ivec2 pos, glm::vec3 expectedColor) {
    glm::vec3 actualColor = board.pixels[pos.x + pos.y * board.width];
    ASSERT_FLOAT_EQ(actualColor.x, expectedColor.x)
          << INFO_VEC2(pos) << INFO_VEC3(expectedColor) << INFO_VEC3(actualColor);
    ASSERT_FLOAT_EQ(actualColor.y, expectedColor.y)
          << INFO_VEC2(pos) << INFO_VEC3(expectedColor) << INFO_VEC3(actualColor);
    ASSERT_FLOAT_EQ(actualColor.z, expectedColor.z)
          << INFO_VEC2(pos) << INFO_VEC3(expectedColor) << INFO_VEC3(actualColor);
}

std::function<void(glm::ivec2)> createNextVisitFunc(Board &board, AStarSolver *solver) {
    return [&board, solver](glm::ivec2 pos) {
        solver->nextStep(board);
        positionEquals(board, pos, visitedColor);
    };
}

TEST(AStarTest, Start_and_Finish_right_next_to_each_other) {
    Board board = initBoard(4, 4);
    setColor(board, {1, 1}, startColor);
    setColor(board, {2, 1}, finishColor);
    auto solver = new AStarSolver();
    solver->nextStep(board);
    solver->nextStep(board);
    ASSERT_TRUE(solver->solved);
}

TEST(AStarTest, Start_and_Finish_one_field_apart) {
    Board board = initBoard(5, 5);
    setColor(board, {1, 1}, startColor);
    setColor(board, {3, 1}, finishColor);

    auto solver = new AStarSolver();
    solver->nextStep(board);
    createNextVisitFunc(board, solver)({2, 1});
    solver->nextStep(board);
    ASSERT_TRUE(solver->solved);
}

TEST(AStarTest, Start_and_Finish_two_straight_fields_apart) {
    Board board = initBoard(6, 6);
    setColor(board, {1, 1}, startColor);
    setColor(board, {4, 1}, finishColor);

    auto solver = new AStarSolver();
    solver->nextStep(board);
    const std::function<void(glm::ivec2)> &assertNextVisit = createNextVisitFunc(board, solver);
    assertNextVisit({2, 1});
    assertNextVisit({3, 1});
    solver->nextStep(board);
    ASSERT_TRUE(solver->solved);
}

TEST(AStarTest, Start_and_Finish_are_separated_by_a_small_wall) {
    Board board = initBoard(6, 6);
    setColor(board, {1, 1}, startColor);
    setColor(board, {2, 0}, obstacleColor);
    setColor(board, {2, 1}, obstacleColor);
    setColor(board, {4, 1}, finishColor);

    auto solver = new AStarSolver();
    solver->nextStep(board);

    const std::function<void(glm::ivec2)> &assertNextVisit = createNextVisitFunc(board, solver);
    assertNextVisit({2, 2});
    assertNextVisit({3, 1});

    solver->nextStep(board);
    ASSERT_TRUE(solver->solved);
}

TEST(AStarTest, Start_and_Finish_are_separated_by_a_big_wall) {
    Board board = initBoard(6, 6);
    setColor(board, {1, 1}, startColor);
    setColor(board, {2, 0}, obstacleColor);
    setColor(board, {2, 1}, obstacleColor);
    setColor(board, {2, 2}, obstacleColor);
    setColor(board, {2, 3}, obstacleColor);
    setColor(board, {4, 1}, finishColor);

    auto solver = new AStarSolver();
    solver->nextStep(board);

    const std::function<void(glm::ivec2)> &assertNextVisit = createNextVisitFunc(board, solver);

    assertNextVisit({1, 0});
    assertNextVisit({1, 2});
    assertNextVisit({0, 1});
    assertNextVisit({0, 0});
    assertNextVisit({0, 2});
    assertNextVisit({1, 3});
    assertNextVisit({0, 3});
    assertNextVisit({2, 4});
    assertNextVisit({3, 3});
    assertNextVisit({4, 2});

    solver->nextStep(board);
    ASSERT_TRUE(solver->solved);
}
