#include "labyrinthwidget.h"
#include <QPainter>
#include <QKeyEvent>

LabyrinthWidget::LabyrinthWidget(QWidget *parent)
    : QWidget(parent), playerRow(1), playerCol(1)
{
    setFocusPolicy(Qt::StrongFocus);
    initMaze();
}

void LabyrinthWidget::initMaze()
{
    // Simple maze: 0 = path, 1 = wall
    int temp[rows][cols] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
        {1,0,1,0,0,0,0,1,0,0,1,0,1,0,1},
        {1,0,1,1,1,1,0,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,1,0,0,0,0,0,0,1,0,1},
        {1,1,1,1,0,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            maze[r][c] = temp[r][c];
}

void LabyrinthWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    int cellW = width() / cols;
    int cellH = height() / rows;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (maze[r][c] == 1) {
                painter.setBrush(Qt::black);
            } else {
                painter.setBrush(Qt::white);
            }
            painter.drawRect(c * cellW, r * cellH, cellW, cellH);
        }
    }
    // Draw player
    painter.setBrush(Qt::red);
    painter.drawEllipse(playerCol * cellW, playerRow * cellH, cellW, cellH);
    // Draw goal
    painter.setBrush(Qt::green);
    painter.drawRect((cols-2) * cellW, (rows-2) * cellH, cellW, cellH);
}

void LabyrinthWidget::keyPressEvent(QKeyEvent *event)
{
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
    // Check for win
    if (playerRow == rows-2 && playerCol == cols-2) {
        setWindowTitle("You Win!");
    }
}

