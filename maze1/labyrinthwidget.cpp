#include "labyrinthwidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTime>
#include "mazegenerator.h"
#include <QInputDialog>

LabyrinthWidget::LabyrinthWidget(QWidget *parent)
    : QWidget(parent), playerRow(1), playerCol(1), playerPixmap(":/asset/apple.png"), portalPixmap(":/asset/portal.png")
{
    setFocusPolicy(Qt::StrongFocus);
    // 添加迷宫大小选择弹窗
    QStringList sizes;
    sizes << "31 x 31" << "63 x 63";
    bool ok = false;
    QString choice = QInputDialog::getItem(this, "选择迷宫大小", "请选择迷宫尺寸:", sizes, 0, false, &ok);
    if (ok && choice == "63 x 63")
    {
        rows = 63;
        cols = 63;
    }
    else
    {
        rows = 31;
        cols = 31;
    }
    portalBRow = rows - 3;
    portalBCol = cols - 3;

    initMaze();
}

void LabyrinthWidget::initMaze()
{
    generateRandomMaze();
    resetPlayer();
}

void LabyrinthWidget::generateRandomMaze()
{
    maze.resize(rows);
    for (int r = 0; r < rows; ++r)
        maze[r].resize(cols);
    MazeGenerator::MazeResult result = MazeGenerator::generate(rows, cols);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            maze[r][c] = result.maze[r][c];
    portalARow = result.portalARow;
    portalACol = result.portalACol;
    portalBRow = result.portalBRow;
    portalBCol = result.portalBCol;
}

void LabyrinthWidget::resetPlayer()
{
    playerRow = 1;
    playerCol = 1;
}

void LabyrinthWidget::setIsPortalWorld(bool portalWorld)
{
    isPortalWorld = portalWorld;
}

void LabyrinthWidget::setReturnPosition(int row, int col)
{
    returnRow = row;
    returnCol = col;
}

void LabyrinthWidget::setSurpriseMode(bool enabled)
{
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
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            if (maze[r][c] == 1)
            {
                painter.setBrush(Qt::black);
            }
            else
            {
                painter.setBrush(Qt::white);
            }
            painter.drawRect(offsetX + c * cellSize, offsetY + r * cellSize, cellSize, cellSize);
        }
    }
    // Draw portals as colored rectangles
    if (surpriseMode)
    {
        if (!isPortalWorld)
        {
            painter.setBrush(QColor(0, 200, 255)); // Cyan for portal A
            painter.drawRect(offsetX + portalACol * cellSize, offsetY + portalARow * cellSize, cellSize, cellSize);
        }
        else
        {
            painter.setBrush(QColor(255, 100, 0)); // Orange for portal B
            painter.drawRect(offsetX + portalBCol * cellSize, offsetY + portalBRow * cellSize, cellSize, cellSize);
        }
    }
    // Draw player as apple (or red ellipse fallback)
    if (!playerPixmap.isNull())
    {
        painter.drawPixmap(offsetX + playerCol * cellSize, offsetY + playerRow * cellSize, cellSize, cellSize, playerPixmap);
    }
    else
    {
        painter.setBrush(Qt::red);
        painter.drawEllipse(offsetX + playerCol * cellSize, offsetY + playerRow * cellSize, cellSize, cellSize);
    }
    // Draw goal (only in main world)
    if (!isPortalWorld)
    {
        painter.setBrush(Qt::green);
        painter.drawRect(offsetX + (cols - 2) * cellSize, offsetY + (rows - 2) * cellSize, cellSize, cellSize);
    }
}

void LabyrinthWidget::setEnabled(bool enabled)
{
    isActive = enabled;
    QWidget::setEnabled(enabled);
}

void LabyrinthWidget::keyPressEvent(QKeyEvent *event)
{
    if (!isActive)
    {
        event->ignore();
        return;
    }
    int newRow = playerRow;
    int newCol = playerCol;
    switch (event->key())
    {
    case Qt::Key_Up:
        newRow--;
        break;
    case Qt::Key_Down:
        newRow++;
        break;
    case Qt::Key_Left:
        newCol--;
        break;
    case Qt::Key_Right:
        newCol++;
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
    if (maze[newRow][newCol] == 0)
    {
        playerRow = newRow;
        playerCol = newCol;
        update();
    }
    // Portal logic: auto-activate when stepping on portal
    if (surpriseMode && !isPortalWorld && playerRow == portalARow && playerCol == portalACol)
    {
        emit requestOpenPortal();
    }
    if (surpriseMode && isPortalWorld && playerRow == portalBRow && playerCol == portalBCol)
    {
        emit requestReturnFromPortal(returnRow, returnCol);
    }
    // Check for win (only in main world)
    if (!isPortalWorld && playerRow == rows - 2 && playerCol == cols - 2)
    {
        setWindowTitle("You Win! Generating new map...");
        generateRandomMaze();
        resetPlayer();
        update();
    }
}

void LabyrinthWidget::removePortal()
{
    portalARow = -1;
    portalACol = -1;
    portalBRow = -1;
    portalBCol = -1;
    update();
}
