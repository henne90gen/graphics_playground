#include <catch.hpp>
#include <glm/glm.hpp>

#include "a_star/AStarSolver.h"

Board initBoard(unsigned int width, unsigned int height) {
    auto pixels = std::vector<glm::vec3>(width * height);
    for (auto &field : pixels) {
        field = {0.0, 0.0, 0.0};
    }
    return {width, height, pixels};
}

void assertEquals(glm::vec3 first, glm::vec3 second) {
    REQUIRE(first.x == Approx(second.x));
    REQUIRE(first.y == Approx(second.y));
    REQUIRE(first.z == Approx(second.z));
}

TEST_CASE("Start and Finish right next to each other") {
    Board board = initBoard(4, 4);
    board.pixels[5] = startColor;
    board.pixels[6] = finishColor;
    auto solver = AStarSolver();
    REQUIRE(solver.nextStep(board));
}

TEST_CASE("Start and Finish one field apart") {
    Board board = initBoard(5, 5);
    board.pixels[6] = startColor;
    board.pixels[8] = finishColor;
    auto solver = AStarSolver();
    solver.nextStep(board);
    assertEquals(board.pixels[7], visitedColor);
    REQUIRE(solver.nextStep(board));
}

TEST_CASE("Start and Finish two straight fields apart") {
    Board board = initBoard(6, 6);
    board.pixels[7] = startColor;
    board.pixels[10] = finishColor;
    auto solver = AStarSolver();
    solver.nextStep(board);
    assertEquals(board.pixels[8], visitedColor);
    solver.nextStep(board);
    assertEquals(board.pixels[9], visitedColor);
    REQUIRE(solver.nextStep(board));
}
