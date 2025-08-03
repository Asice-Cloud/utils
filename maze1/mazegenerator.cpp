#include <queue>
#include "mazegenerator.h"
#include <functional>

bool MazeGenerator::isSolvable(const std::vector<std::vector<int>> &maze, int startRow, int startCol, int endRow, int endCol)
{
    int rows = maze.size();
    if (rows == 0)
        return false;
    int cols = maze[0].size();
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::queue<std::pair<int, int>> q;
    q.push({startRow, startCol});
    visited[startRow][startCol] = true;
    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};
    while (!q.empty())
    {
        auto [r, c] = q.front();
        q.pop();
        if (r == endRow && c == endCol)
            return true;
        for (int d = 0; d < 4; ++d)
        {
            int nr = r + dr[d], nc = c + dc[d];
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && maze[nr][nc] == 0 && !visited[nr][nc])
            {
                visited[nr][nc] = true;
                q.push({nr, nc});
            }
        }
    }
    return false;
}

MazeGenerator::MazeResult MazeGenerator::generate(int rows, int cols)
{
    MazeResult result;
    result.maze.resize(rows, std::vector<int>(cols, 1));
    std::random_device rd;
    std::mt19937 gen(rd());

    // Aldous-Broder algorithm
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result.maze[r][c] = 1;
    for (int r = 1; r < rows - 1; ++r)
        for (int c = 1; c < cols - 1; ++c)
            if (r % 2 == 1 && c % 2 == 1)
                result.maze[r][c] = 0;
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    int totalCells = 0;
    for (int r = 1; r < rows - 1; r += 2)
        for (int c = 1; c < cols - 1; c += 2)
            ++totalCells;
    int visitedCells = 1;
    int cr = 1, cc = 1;
    visited[cr][cc] = true;
    static const int dr[4] = {-2, 2, 0, 0};
    static const int dc[4] = {0, 0, -2, 2};
    while (visitedCells < totalCells)
    {
        std::vector<int> dirs = {0, 1, 2, 3};
        std::shuffle(dirs.begin(), dirs.end(), gen);
        bool moved = false;
        for (int d : dirs)
        {
            int nr = cr + dr[d], nc = cc + dc[d];
            if (nr > 0 && nr < rows - 1 && nc > 0 && nc < cols - 1 && nr % 2 == 1 && nc % 2 == 1)
            {
                if (!visited[nr][nc])
                {
                    result.maze[(cr + nr) / 2][(cc + nc) / 2] = 0;
                    visited[nr][nc] = true;
                    ++visitedCells;
                }
                cr = nr;
                cc = nc;
                moved = true;
                break;
            }
        }
        if (!moved)
        {
            // If stuck, pick a random visited cell
            std::vector<std::pair<int, int>> vCells;
            for (int r = 1; r < rows - 1; r += 2)
                for (int c = 1; c < cols - 1; c += 2)
                    if (visited[r][c])
                        vCells.emplace_back(r, c);
            auto pick = vCells[std::uniform_int_distribution<>(0, vCells.size() - 1)(gen)];
            cr = pick.first;
            cc = pick.second;
        }
    }
    // Ensure start and end are open
    result.maze[1][1] = 0;
    result.maze[rows - 2][cols - 2] = 0;

    // Find a path from start to goal
    std::vector<std::pair<int, int>> path;
    std::vector<std::vector<bool>> visited_dfs(rows, std::vector<bool>(cols, false));
    std::function<bool(int, int)> dfs = [&](int r, int c)
    {
        if (r == rows - 2 && c == cols - 2)
        {
            path.emplace_back(r, c);
            return true;
        }
        visited_dfs[r][c] = true;
        static const int dr[4] = {-1, 1, 0, 0};
        static const int dc[4] = {0, 0, -1, 1};
        for (int d = 0; d < 4; ++d)
        {
            int nr = r + dr[d], nc = c + dc[d];
            if (nr > 0 && nr < rows - 1 && nc > 0 && nc < cols - 1 && result.maze[nr][nc] == 0 && !visited_dfs[nr][nc])
            {
                if (dfs(nr, nc))
                {
                    path.emplace_back(r, c);
                    return true;
                }
            }
        }
        return false;
    };
    dfs(1, 1);
    std::reverse(path.begin(), path.end());
    // Place portals on the path (not at start or goal)
    if (path.size() > 4)
    {
        std::vector<size_t> validIndices;
        for (size_t i = 1; i < path.size() - 1; ++i)
        {
            int r = path[i].first, c = path[i].second;
            if (result.maze[r][c] == 0)
            {
                validIndices.push_back(i);
            }
        }
        if (validIndices.size() >= 2)
        {
            std::shuffle(validIndices.begin(), validIndices.end(), gen);
            size_t idxA = validIndices[0];
            size_t idxB = validIndices[1];
            result.portalARow = path[idxA].first;
            result.portalACol = path[idxA].second;
            result.portalBRow = path[idxB].first;
            result.portalBCol = path[idxB].second;
        }
        else if (validIndices.size() == 1)
        {
            result.portalARow = path[validIndices[0]].first;
            result.portalACol = path[validIndices[0]].second;
            result.portalBRow = path[validIndices[0]].first;
            result.portalBCol = path[validIndices[0]].second;
        }
    }
    return result;
}
