#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "rastermatrix.h"
#include "mylabel.h"
#include "geoimage.h"
#include <tiffio.h>
#include <QMainWindow>
#include <QLabel>
#include <QStatusBar>
#include <QImage>
#include <QStringList>
#include <QMdiSubWindow>
#include "progress.h"

const int maxScale = 4;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Progress* progress;

private slots:

    void on_bIncrease_clicked();

    void on_bDecrease_clicked();

    void on_mdiArea_subWindowActivated(QMdiSubWindow *arg1);

    void on_actionOpen_triggered();

    void on_actionSaveRPC_triggered();

    void on_actionAccuracy_triggered();

    void on_action2Images_triggered();

    void on_actionHistogram_triggered();

    void on_actionExit_triggered();

    void on_actionSave_triggered();

private:
    Ui::MainWindow *ui;
    void writeRPC(Rpc const &rpc);
    bool getGeoImage(GeoImage& img, QMdiSubWindow* subWindow);
    MyLabel* getLabel(QMdiSubWindow* subWindow);
    bool openImg(GeoImage& img, QString& fileName);
    void addWindow(GeoImage& img, QString fileName);
    void transformImages(GeoImage& img1, GeoImage& img2, const QString& img1name, const QString& img2name, double H);
    void setImageToWindow(GeoImage &img, const QString& fileName, QMdiSubWindow* subWindow = 0);
    bool isActivateWindow();
    void printToConsole(const QString& str);
};

#endif // MAINWINDOW_H
