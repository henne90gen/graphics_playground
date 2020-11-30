#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <glm/glm.hpp>

#include "../../test/TestUtils.h"
#include "AStarSolver.h"

int main(int argc, char *argv[]) {
    return Catch::Session().run(argc, argv);
}

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
    INFO_VEC2(pos)
    INFO_VEC3(expectedColor)
    INFO_VEC3(actualColor)
    REQUIRE(actualColor.x == Approx(expectedColor.x));
    REQUIRE(actualColor.y == Approx(expectedColor.y));
    REQUIRE(actualColor.z == Approx(expectedColor.z));
}

std::function<void(glm::ivec2)> createNextVisitFunc(Board &board, AStarSolver *solver) {
    return [&board, solver](glm::ivec2 pos) {
        solver->nextStep(board);
        positionEquals(board, pos, visitedColor);
    };
}

TEST_CASE("Start and Finish right next to each other") {
    Board board = initBoard(4, 4);
    setColor(board, {1, 1}, startColor);
    setColor(board, {2, 1}, finishColor);
    auto solver = new AStarSolver();
    solver->nextStep(board);
    solver->nextStep(board);
    REQUIRE(solver->solved);
}

TEST_CASE("Start and Finish one field apart") {
    Board board = initBoard(5, 5);
    setColor(board, {1, 1}, startColor);
    setColor(board, {3, 1}, finishColor);

    auto solver = new AStarSolver();
    solver->nextStep(board);
    createNextVisitFunc(board, solver)({2, 1});
    solver->nextStep(board);
    REQUIRE(solver->solved);
}

TEST_CASE("Start and Finish two straight fields apart") {
    Board board = initBoard(6, 6);
    setColor(board, {1, 1}, startColor);
    setColor(board, {4, 1}, finishColor);

    auto solver = new AStarSolver();
    solver->nextStep(board);
    const std::function<void(glm::ivec2)> &assertNextVisit = createNextVisitFunc(board, solver);
    assertNextVisit({2, 1});
    assertNextVisit({3, 1});
    solver->nextStep(board);
    REQUIRE(solver->solved);
}

TEST_CASE("Start and Finish are separated by a small wall") {
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
    REQUIRE(solver->solved);
}

TEST_CASE("Start and Finish are separated by a big wall") {
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
    REQUIRE(solver->solved);
}
