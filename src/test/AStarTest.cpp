#include <catch.hpp>
#include <glm/glm.hpp>

#include "a_star/AStarSolver.h"

void resetBoard(std::vector<glm::vec3> &board) {
    for (auto &field : board) {
        field = {0.0, 0.0, 0.0};
    }
}

bool assertEquals(glm::vec3 first, glm::vec3 second) {
    REQUIRE(first.x == second.x);
    REQUIRE(first.y == second.y);
    REQUIRE(first.z == second.z);
}

TEST_CASE("Start and Finish right next to each other") {
    auto board = std::vector<glm::vec3>(4 * 4);
    board[5] = startColor;
    board[6] = finishColor;
    auto solver = AStarSolver();
    solver.nextStep(board, 4, 4);
}

TEST_CASE("Start and Finish one field apart") {
    auto board = std::vector<glm::vec3>(5 * 5);
    board[6] = startColor;
    board[8] = finishColor;
    auto solver = AStarSolver();
    solver.nextStep(board, 5, 5);
    assertEquals(board[7], pathColor);
}
