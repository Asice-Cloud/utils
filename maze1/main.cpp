#include <QApplication>
#include <QDebug>
#include <QPointer>
#include <QMessageBox>
#include "labyrinthwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Ask the player if they want to enable surprise mode
    bool surpriseMode = false;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Game Mode");
    msgBox.setText("Do you want to enable Surprise Mode? (Portals will appear if enabled)");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes)
        surpriseMode = true;

    LabyrinthWidget *mainWindow = new LabyrinthWidget();
    mainWindow->setSurpriseMode(surpriseMode);
    mainWindow->setWindowTitle("Labyrinth Game");
    mainWindow->resize(600, 400);
    mainWindow->show();

    QPointer<LabyrinthWidget> portalWindow = nullptr;

    QObject::connect(mainWindow, &LabyrinthWidget::requestOpenPortal, [&]()
                     {
        if (!mainWindow->isSurpriseMode()) return;
        if (portalWindow) return;
        mainWindow->setEnabled(false);
        portalWindow = new LabyrinthWidget();
        portalWindow->setSurpriseMode(true);
        portalWindow->setIsPortalWorld(true);
        portalWindow->setWindowTitle("Limbo 嘻嘻,我一定要活下去");
        portalWindow->resize(600, 400);
        portalWindow->setReturnPosition(mainWindow->getPortalBRow(), mainWindow->getPortalBCol());
        portalWindow->show();
        QObject::connect(portalWindow, &LabyrinthWidget::requestReturnFromPortal, mainWindow, [&](int row, int col) {
            mainWindow->setPlayerRow(row);
            mainWindow->setPlayerCol(col);
            mainWindow->removePortal(); // Remove portal after returning
            mainWindow->update();
            mainWindow->setEnabled(true);
            portalWindow->close();
            portalWindow->deleteLater();
            portalWindow = nullptr;
        }); });

    return a.exec();
}