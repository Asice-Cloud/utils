#ifndef LABYRINTHWIDGET_H
#define LABYRINTHWIDGET_H

#include <QWidget>
#include "mazegenerator.h"
#include <QPixmap>
#include <QPointer>

class LabyrinthWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LabyrinthWidget(QWidget *parent = nullptr);
    void setIsPortalWorld(bool portalWorld);
    void setReturnPosition(int row, int col);
    int getPortalBRow() const { return portalBRow; }
    int getPortalBCol() const { return portalBCol; }
    int getPlayerRow() const { return playerRow; }
    int getPlayerCol() const { return playerCol; }
    void setPlayerRow(int row) { playerRow = row; }
    void setPlayerCol(int col) { playerCol = col; }
    void setEnabled(bool enabled);
    void setSurpriseMode(bool enabled);
    bool isSurpriseMode() const { return surpriseMode; }
    void removePortal();

signals:
    void requestOpenPortal();
    void requestReturnFromPortal(int row, int col);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    int rows;
    int cols;
    std::vector<std::vector<int>> maze;
    int playerRow;
    int playerCol;
    QPixmap playerPixmap;
    QPixmap portalPixmap;
    bool isPortalWorld = false;
    int portalARow = 2, portalACol = 2; // Example position for portal A
    int portalBRow, portalBCol;         // Example position for portal B
    int returnRow = 1, returnCol = 1;   // Where to return in original map
    void initMaze();
    void generateRandomMaze();
    void resetPlayer();
    bool isActive = true;
    bool surpriseMode = false;
};

#endif // LABYRINTHWIDGET_H
