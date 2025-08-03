#pragma once
#include <vector>
#include <utility>
#include <random>
#include <stack>
#include <algorithm>

class MazeGenerator
{
public:
    struct MazeResult
    {
        std::vector<std::vector<int>> maze;
        int portalARow = -1, portalACol = -1;
        int portalBRow = -1, portalBCol = -1;
    };

    static MazeResult generate(int rows, int cols);
    static bool isSolvable(const std::vector<std::vector<int>> &maze, int startRow, int startCol, int endRow, int endCol);
};
