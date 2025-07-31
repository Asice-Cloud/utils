#include "labyrinthwidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTime>
#include <random>
#include <stack>
#include <vector>
#include <algorithm>

LabyrinthWidget::LabyrinthWidget(QWidget *parent)
    : QWidget(parent), playerRow(1), playerCol(1), playerPixmap(":/asset/apple.png"), portalPixmap(":/asset/portal.png")
{
    setFocusPolicy(Qt::StrongFocus);
    initMaze();
}

void LabyrinthWidget::initMaze()
{
    generateRandomMaze();
    resetPlayer();
}

void LabyrinthWidget::generateRandomMaze()
{
    // Initialize all cells as walls
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            maze[r][c] = 1;

    // DFS maze generation
    struct Cell { int r, c; };
    std::stack<Cell> stack;
    std::random_device rd;
    std::mt19937 gen(rd());

    // Start at (1,1)
    maze[1][1] = 0;
    stack.push({1, 1});

    auto neighbors = [&](int r, int c) {
        std::vector<std::pair<int, int>> nbs;
        const int dr[4] = {-2, 2, 0, 0};
        const int dc[4] = {0, 0, -2, 2};
        for (int d = 0; d < 4; ++d) {
            int nr = r + dr[d], nc = c + dc[d];
            if (nr > 0 && nr < rows-1 && nc > 0 && nc < cols-1 && maze[nr][nc] == 1)
                nbs.emplace_back(nr, nc);
        }
        std::shuffle(nbs.begin(), nbs.end(), gen);
        return nbs;
    };

    while (!stack.empty()) {
        auto [r, c] = stack.top();
        auto nbs = neighbors(r, c);
        if (!nbs.empty()) {
            auto [nr, nc] = nbs.front();
            // Remove wall between current and neighbor
            maze[(r+nr)/2][(c+nc)/2] = 0;
            maze[nr][nc] = 0;
            stack.push({nr, nc});
        } else {
            stack.pop();
        }
    }
    // Ensure start and end are open
    maze[1][1] = 0;
    maze[rows-2][cols-2] = 0;

    // Find all valid road cells (excluding start and goal)
    std::vector<std::pair<int, int>> roadCells;
    for (int r = 1; r < rows-1; ++r) {
        for (int c = 1; c < cols-1; ++c) {
            if (maze[r][c] == 0 && !(r == 1 && c == 1) && !(r == rows-2 && c == cols-2)) {
                roadCells.emplace_back(r, c);
            }
        }
    }
    // Find a path from start to goal
    std::vector<std::pair<int, int>> path;
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::function<bool(int, int)> dfs = [&](int r, int c) {
        if (r == rows-2 && c == cols-2) {
            path.emplace_back(r, c);
            return true;
        }
        visited[r][c] = true;
        static const int dr[4] = {-1, 1, 0, 0};
        static const int dc[4] = {0, 0, -1, 1};
        for (int d = 0; d < 4; ++d) {
            int nr = r + dr[d], nc = c + dc[d];
            if (nr > 0 && nr < rows-1 && nc > 0 && nc < cols-1 && maze[nr][nc] == 0 && !visited[nr][nc]) {
                if (dfs(nr, nc)) {
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
    if (path.size() > 4) {
        // Place portalA and portalB at random positions along the path (not start or goal)
        std::vector<size_t> validIndices;
        for (size_t i = 1; i < path.size() - 1; ++i) {
            int r = path[i].first, c = path[i].second;
            if (maze[r][c] == 0) {
                validIndices.push_back(i);
            }
        }
        if (validIndices.size() >= 2) {
            std::shuffle(validIndices.begin(), validIndices.end(), gen);
            size_t idxA = validIndices[0];
            size_t idxB = validIndices[1];
            portalARow = path[idxA].first;
            portalACol = path[idxA].second;
            portalBRow = path[idxB].first;
            portalBCol = path[idxB].second;
        } else if (validIndices.size() == 1) {
            portalARow = path[validIndices[0]].first;
            portalACol = path[validIndices[0]].second;
            portalBRow = path[validIndices[0]].first;
            portalBCol = path[validIndices[0]].second;
        }
    }
}

void LabyrinthWidget::resetPlayer()
{
    playerRow = 1;
    playerCol = 1;
}

void LabyrinthWidget::setIsPortalWorld(bool portalWorld) {
    isPortalWorld = portalWorld;
}

void LabyrinthWidget::setReturnPosition(int row, int col) {
    returnRow = row;
    returnCol = col;
}

void LabyrinthWidget::setSurpriseMode(bool enabled) {
    surpriseMode = enabled;
}

void LabyrinthWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    int cellW = width() / cols;
    int cellH = height() / rows;
    int cellSize = std::min(cellW, cellH);
    int offsetX = (width() - cellSize * cols) / 2;
    int offsetY = (height() - cellSize * rows) / 2;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (maze[r][c] == 1) {
                painter.setBrush(Qt::black);
            } else {
                painter.setBrush(Qt::white);
            }
            painter.drawRect(offsetX + c * cellSize, offsetY + r * cellSize, cellSize, cellSize);
        }
    }
    // Draw portals as colored rectangles
    if (surpriseMode) {
        if (!isPortalWorld) {
            painter.setBrush(QColor(0, 200, 255)); // Cyan for portal A
            painter.drawRect(offsetX + portalACol * cellSize, offsetY + portalARow * cellSize, cellSize, cellSize);
        } else {
            painter.setBrush(QColor(255, 100, 0)); // Orange for portal B
            painter.drawRect(offsetX + portalBCol * cellSize, offsetY + portalBRow * cellSize, cellSize, cellSize);
        }
    }
    // Draw player as apple (or red ellipse fallback)
    if (!playerPixmap.isNull()) {
        painter.drawPixmap(offsetX + playerCol * cellSize, offsetY + playerRow * cellSize, cellSize, cellSize, playerPixmap);
    } else {
        painter.setBrush(Qt::red);
        painter.drawEllipse(offsetX + playerCol * cellSize, offsetY + playerRow * cellSize, cellSize, cellSize);
    }
    // Draw goal (only in main world)
    if (!isPortalWorld) {
        painter.setBrush(Qt::green);
        painter.drawRect(offsetX + (cols-2) * cellSize, offsetY + (rows-2) * cellSize, cellSize, cellSize);
    }
}

void LabyrinthWidget::setEnabled(bool enabled) {
    isActive = enabled;
    QWidget::setEnabled(enabled);
}

void LabyrinthWidget::keyPressEvent(QKeyEvent *event)
{
    if (!isActive) {
        event->ignore();
        return;
    }
    int newRow = playerRow;
    int newCol = playerCol;
    switch (event->key()) {
    case Qt::Key_Up:    newRow--; break;
    case Qt::Key_Down:  newRow++; break;
    case Qt::Key_Left:  newCol--; break;
    case Qt::Key_Right: newCol++; break;
    default: QWidget::keyPressEvent(event); return;
    }
    if (maze[newRow][newCol] == 0) {
        playerRow = newRow;
        playerCol = newCol;
        update();
    }
    // Portal logic: auto-activate when stepping on portal
    if (surpriseMode && !isPortalWorld && playerRow == portalARow && playerCol == portalACol) {
        emit requestOpenPortal();
    }
    if (surpriseMode && isPortalWorld && playerRow == portalBRow && playerCol == portalBCol) {
        emit requestReturnFromPortal(returnRow, returnCol);
    }
    // Check for win (only in main world)
    if (!isPortalWorld && playerRow == rows-2 && playerCol == cols-2) {
        setWindowTitle("You Win! Generating new map...");
        generateRandomMaze();
        resetPlayer();
        update();
    }
}
