#ifndef LABYRINTHWIDGET_H
#define LABYRINTHWIDGET_H

#include <QWidget>

class LabyrinthWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LabyrinthWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    static const int rows = 10;
    static const int cols = 15;
    int maze[rows][cols];
    int playerRow;
    int playerCol;
    void initMaze();
};

#endif // LABYRINTHWIDGET_H

