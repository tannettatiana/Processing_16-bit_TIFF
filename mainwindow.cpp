#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "accuracytable.h"
#include "ui_accuracytable.h"
#include "transformimagesdialog.h"
#include "ui_transformimagesdialog.h"
#include <iostream>
#include <fstream>
#include <limits>
#include <QPixmap>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QString>
#include <QFileDialog>
#include <QColor>
#include <QRgb>
#include <QtMath>
#include <QtNumeric>
#include <QException>
#include <QMessageBox>
#include <QMouseEvent>
#include <QCursor>
#include <QScrollArea>
#include <QVector>
#include <QMdiSubWindow>
#include <QDebug>
#include <cmath>
#include <iomanip>
#include "histogram.h"
#include "ui_histogram.h"
#include <QProgressBar>

#pragma pack (push, 1)
struct RGB{
    uint16_t blue = 0;
    uint16_t green = 0;
    uint16_t red = 0;
};
#pragma pack (pop)


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMouseTracking(true);
    ui->statusBar->addWidget(ui->lb2);
    ui->statusBar->addWidget(ui->lbBrights);
    ui->statusBar->addWidget(ui->geoLabel);

    progress = new Progress();
    ui->statusBar->addWidget(progress, 1);

//    setStatusBar(bar);

}

MainWindow::~MainWindow()
{
    delete ui;
}

double roundToHalf(double x){
    return std::floor(x) + 0.5;
}

//определение выхода пикселя с заданными координатами за рамки изображения
uint16_t detectionOutOfBoundBW(const int& width, const int& height, const int& X, const int& Y, RasterMatrix<uint16_t>& I){
    uint16_t res = 0;
    if (Y >= height || Y < 0 || X >= width || X < 0) {
        res = 0;
    } else {
        res = I[Y][X];
    }
    return res;
}

//определение яркости пикселя по 4 соседним для билинейной интерполяции
uint16_t countBright(const int& width, const int& height, const int& X1, const int& X2, const int& Y1, const int& Y2, const double& dx, const double& dy, RasterMatrix<uint16_t>& IOrig, bool& isNull){
    QVector<uint16_t> I(4, 0);
    I[0] = detectionOutOfBoundBW(width, height, X1, Y1, IOrig);
    I[1] = detectionOutOfBoundBW(width, height, X2, Y1, IOrig);
    I[2] = detectionOutOfBoundBW(width, height, X1, Y2, IOrig);
    I[3] = detectionOutOfBoundBW(width, height, X2, Y2, IOrig);

    int k = 0;
    for (int i = 0; i < 4; i++){
        if (I[i] == 0){
            k++;
        }
    }

    uint16_t res = 0;
    if (k < 1){
        res = std::round(I[0] * (1 - dx) * (1 - dy) + I[1] * (dx) * (1 - dy) + I[2] * (1 - dx) * (dy) + I[3] * (dx) * (dy));
        isNull = false;
    } else {
        isNull = true;
    }

    return res;
}

//билинейная интерполяция
template <typename Type>
bool BLinterpolation(RasterMatrix<uint16_t>& rasterMatr, double x, double y, Type& res){

    double xOrig = x;
    double yOrig = y;

    x = roundToHalf(xOrig);
    y = roundToHalf(yOrig);
    int xres1, xres2, yres1, yres2;
    if (std::round(xOrig) > x){
        xres1 = int(x);
        xres2 = int(x) + 1;
    }
    else{
        xres1 = int(x) - 1;
        xres2 = int(x);
    }

    if (std::round(yOrig) > y){
        yres1 = int(y);
        yres2 = int(y) + 1;
    }
    else{
        yres2 = int(y);
        yres1 = int(y) - 1;
    }
    double dx = xOrig - xres1 - 0.5;
    double dy = yOrig - yres1 - 0.5;
    bool isNull = true;
    res = countBright(rasterMatr.getColumn(), rasterMatr.getRow(), xres1, xres2, yres1, yres2, dx, dy, rasterMatr, isNull);

    return !isNull;
}

//добавление окна в MdiArea
void MainWindow::addWindow(GeoImage& img, QString fileName){
    QScrollArea* scroll = new QScrollArea();
    MyLabel* labelPixmap = new MyLabel(img);
    labelPixmap->setCoordinatesLabel(ui->lb2, ui->lbBrights, ui->geoLabel);
    QPixmap qMap = QPixmap::fromImage(img.histImg);
    labelPixmap->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    labelPixmap->setPixmap(qMap);

    scroll->setWidget(labelPixmap);
    scroll->setMouseTracking(true);
    if (!qMap.isNull()){
        QWidget *widget = new QWidget();
        QGridLayout *gridLayout = new QGridLayout();
        gridLayout->addWidget(scroll);
        widget->setLayout(gridLayout);
        QMdiSubWindow *subWindow1 = new QMdiSubWindow;
        subWindow1->setAttribute(Qt::WA_DeleteOnClose,true);
        subWindow1->setWidget(widget);
        ui->mdiArea->addSubWindow(subWindow1);
        widget->setMouseTracking(true);
        widget->setWindowTitle(fileName);
        widget->show();
    } else {
        QMessageBox::critical(this, "Ошибка","Открываесое изображение не существует!");
    }
}

bool MainWindow::openImg(GeoImage& img, QString& fileName){
    int wasRead = img.load(fileName);
    if (wasRead == 0){
        addWindow(img, fileName);
    } else if (wasRead == -2){
        QMessageBox::critical(this, "Ошибка", img.brightnessErr);
        return false;
    } else {
        QMessageBox::critical(this, "Ошибка","Файл не был открыт");
        return false;
    }

    return true;

}

//запись RPC в файл
void MainWindow::writeRPC(Rpc const &rpc){
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить данные RPC"), QCoreApplication::applicationDirPath(), tr("Text files (*.txt)"));
    QByteArray ba = fileName.toLocal8Bit();
    const char *outFileName = ba.data();

    std::ofstream out;
    out.open(outFileName);

    if (!out){
        QMessageBox::critical(this, "Ошибка", "Файл не был создан!");
        return;
    }

    out << "LINE_OFF: " << rpc.data.LINE_OFF << " pixels" << std::endl;
    out << "SAMP_OFF: " << rpc.data.SAMP_OFF << " pixels" << std::endl;
    out << "LAT_OFF: " << QString("%1").arg(rpc.data.LAT_OFF, 10,'f', 20,'0').toLocal8Bit().data() << " degrees" << std::endl;
    out << "LONG_OFF: " << QString("%1").arg(rpc.data.LONG_OFF, 10,'f', 20,'0').toLocal8Bit().data() << " degrees" << std::endl;
    out << "HEIGHT_OFF: " << QString("%1").arg(rpc.data.HEIGHT_OFF, 10,'f', 20,'0').toLocal8Bit().data() << " meters" << std::endl;
    out << "LINE_SCALE: " << QString("%1").arg(rpc.data.LINE_SCALE, 10,'f', 20,'0').toLocal8Bit().data() << " pixels" << std::endl;
    out << "SAMP_SCALE: " << QString("%1").arg(rpc.data.SAMP_SCALE, 10,'f', 20,'0').toLocal8Bit().data() << " pixels" << std::endl;
    out << "LAT_SCALE: " << QString("%1").arg(rpc.data.LAT_SCALE, 10,'f', 20,'0').toLocal8Bit().data() << " degrees" << std::endl;
    out << "LONG_SCALE: " << QString("%1").arg(rpc.data.LONG_SCALE , 10,'f', 20,'0').toLocal8Bit().data() << " degrees" << std::endl;
    out << "HEIGHT_SCALE: " << QString("%1").arg(rpc.data.HEIGHT_SCALE, 10,'f', 20,'0').toLocal8Bit().data() << " meters" << std::endl;
    for (int i = 0; i < 20; i++){
        out << "LINE_NUM_COEFF_" << i + 1 << ": " << QString("%1").arg(rpc.data.LINE_NUM_COEFF[i], 10,'f', 20,'0').toLocal8Bit().data() << std::endl;
    }
    for (int i = 0; i < 20; i++){
        out << "LINE_DEN_COEFF_" << i + 1 << ": " << QString("%1").arg(rpc.data.LINE_DEN_COEFF[i], 10,'f', 20,'0').toLocal8Bit().data() << std::endl;
    }
    for (int i = 0; i < 20; i++){
        out << "SAMP_NUM_COEFF_" << i + 1 << ": " << QString("%1").arg(rpc.data.SAMP_NUM_COEFF[i], 10,'f', 20,'0').toLocal8Bit().data() << std::endl;
    }
    for (int i = 0; i < 20; i++){
        out << "SAMP_DEN_COEFF_" << i + 1 << ": " << QString("%1").arg(rpc.data.SAMP_DEN_COEFF[i], 10,'f', 20,'0').toLocal8Bit().data() << std::endl;
    }
    out.close();
}

//получение изображения из окна
bool MainWindow::getGeoImage(GeoImage& img, QMdiSubWindow* subWindow){
    try{
        QWidget* widget = subWindow -> widget();
        QGridLayout* gridLayout = qobject_cast<QGridLayout*>(widget -> layout());
        QScrollArea* scroll = qobject_cast<QScrollArea*>(gridLayout -> itemAtPosition(0, 0) -> widget());
        QWidget* obj = qobject_cast<QWidget*>(scroll -> widget());
        MyLabel* labelPixmap = qobject_cast<MyLabel*>(obj);
        img = labelPixmap -> getGeoImage();
    } catch (QException& e){
        return false;
    }
    return true;
}

//получение объекта myLabel из окна
MyLabel* MainWindow::getLabel(QMdiSubWindow* subWindow){
    QWidget* widget = subWindow -> widget();
    QGridLayout* gridLayout = qobject_cast<QGridLayout*>(widget -> layout());
    QScrollArea* scroll = qobject_cast<QScrollArea*>(gridLayout -> itemAtPosition(0, 0) -> widget());
    QWidget* obj = qobject_cast<QWidget*>(scroll -> widget());
    return qobject_cast<MyLabel*>(obj);
}

//вычисление x, y, z
QVector<double> calcXYZ(double a, double b, double B, double L, double H, double BDeg, double LDeg){
    QVector<double> xyz(3, 0);
    double alpha = (a - b) / a; //сжатие эллипсоида
    double e2 = 2 * alpha - alpha * alpha; //квадрат эксцентриситета
    double N = a / (qSqrt(1 - e2 * qSin(B) * qSin(B)));
    xyz[0] = (N + H) * qCos(B) * qCos(L);
    xyz[1] = (N + H) * qCos(B) * qSin(L);
    xyz[2] = ((1 - e2) * N + H) * qSin(B);

    return xyz;
}

//вычисление в радианах B и L
bool calcBL(double& B, double& L, double& BDeg, double& LDeg){

    //приведение к виду от -180 до 180
    if (BDeg < -90 || BDeg > 90 || LDeg < -180 || LDeg > 180){
        return false;
    }
    B = BDeg * M_PI / 180;

    L = LDeg * M_PI / 180;
    return true;
}

//вычисление вектора значений x, y, z по B, L, H
void calcP(Matrix<double>& XYZ, double& BVecI, double& LVecI, double& HVecI,
            double B, double L, int i){
    const double a = 6378137;
    const double b = 6356752.314;
    QVector<double> tempXYZ = calcXYZ(a, b, B, L, HVecI, BVecI, LVecI);
    for (int j = 0; j < 3; j++){
        XYZ[i][j] = tempXYZ[j];
    }
}

//вычисление разности R в м
double calcR(Matrix<double>& XYZ, Matrix<double>& XYZ2, int i){
    return sqrt(pow((XYZ[i][0] - XYZ2[i][0]), 2) + pow((XYZ[i][1] - XYZ2[i][1]), 2) + pow((XYZ[i][2] - XYZ2[i][2]), 2));
}

//вычисление общей оценки точности
double calcRMS(QVector<double>& R){
    double res = 0;
    for (int i = 0; i < R.length(); i++){
        res += R[i] * R[i];
    }
    res /= R.length();
    return sqrt(res);
}

//наложение двух изображений
void MainWindow::transformImages(GeoImage& img1, GeoImage& img2, const QString& img1name, const QString& img2name, double H){
    if (img1.originMatrix.getColorsCount() == 1){
        try{
            progress->start("Наложение двух изображений: ");

            RasterMatrix<uint16_t> resImg = img1.originMatrix;
            double per = 0;
            double length = img1.originMatrix.getRow();
            for (int i = 0; i < img1.originMatrix.getRow(); i++){
                for (int j = 0; j < img1.originMatrix.getColumn(); j++){
                    double B = 0;
                    double L = 0;
                    int k = 0;
                    img1.rpc.planar2geo(j + 0.5, i + 0.5, H, B, L, k);
                    double x, y;
                    img2.rpc.geo2planar(B, L, H, y, x);
                    if ((x < img2.originMatrix.getColumn()) && (y < img2.originMatrix.getRow())
                           && (x >= 0) && (y >= 0)){
                        uint16_t res = 0;
                        bool isNotNullPix = BLinterpolation<uint16_t>(img2.originMatrix, x, y, res);
                        if (isNotNullPix){
                            resImg[i][j] = res;
                        }
                    }
                }
                per = i / (double)(length) * 100;
                if (progress->set_progress(per)){
                    progress->stop();
                    return;
                }
            }

            progress->stop();

            GeoImage img(progress);
            int err = img.loadFromMatr(resImg);
            if (err == 0){
                img.rpc = img1.rpc;
                addWindow(img, "Изображение " + img2name + ", наложенное на изображение" + img1name);
            } else if (err == -2){
                QMessageBox::critical(this, "Ошибка!", img.brightnessErr);
            } else {
                QMessageBox::critical(this, "Ошибка!", "Не удалось наложить изображения!");
            }

        } catch (const std::runtime_error& e){
            printToConsole(QString("Ошибка в процессе наложения изображений!") + QString(e.what()));
//            std::cout << "Error in transform images process! " << e.what() << std::endl;
            return;
        }

    } else {    //colorsCount == 3
        QMessageBox::critical(this, "Ошибка!", "Поддержка трехканальных изображений не осуществляется!");
    }
}

//установка изображения в окно
void MainWindow::setImageToWindow(GeoImage &img, const QString& fileName, QMdiSubWindow* subWindow){

    QPixmap qMap = QPixmap::fromImage(img.scaledImage);

    if (qMap.isNull()){
        QMessageBox::critical(this, "Ошибка","Не удалось установить изображение!");
        return;
    }

    if (subWindow == Q_NULLPTR){
        QScrollArea* scroll = new QScrollArea();
        MyLabel* labelPixmap = new MyLabel(img);
        labelPixmap->setCoordinatesLabel(ui->lb2, ui->lbBrights, ui->geoLabel);
        labelPixmap->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        labelPixmap->setPixmap(qMap);
        scroll->setWidget(labelPixmap);
        scroll->setMouseTracking(true);
        QWidget *widget = new QWidget();
        QGridLayout *gridLayout = new QGridLayout();
        gridLayout->addWidget(scroll);
        widget->setLayout(gridLayout);
        QMdiSubWindow *subWindow1 = new QMdiSubWindow;
        subWindow1->setAttribute(Qt::WA_DeleteOnClose,true);
        subWindow1->setWidget(widget);
        subWindow1->setWindowTitle(fileName);
        ui->mdiArea->addSubWindow(subWindow1);
        widget->show();
    } else {
        MyLabel* label = getLabel(subWindow);
        label->setImage(img);
        label->setPixmap(qMap);
        label->resize(qMap.width(), qMap.height());
    }

}

//выбрано ли окно
bool MainWindow::isActivateWindow(){
    if (ui -> mdiArea -> activeSubWindow() == Q_NULLPTR){
        QMessageBox::critical(this, "Ошибка", "Выберите окно с изображением!");
        return false;
    } else {
        return true;
    }
}

//увеличение масштаба
void MainWindow::on_bIncrease_clicked()
{
    GeoImage img(progress);
    if (!isActivateWindow()) {
        QMessageBox::critical(this, "Ошибка", "Выберите окно!");
        return;
    }

    QMdiSubWindow* window = ui->mdiArea->activeSubWindow();
    if (!getGeoImage(img, window)){
        QMessageBox::critical(this, "Ошибка", "Невозможно получить изображение из окна!");
        return;
    }

    if (img.scale + 1 > maxScale){
        return;
    }

    img.scale += 1;
    img.scale = img.scale == 0 ? 2 : img.scale;
    if (std::fabs(img.scale) == 1){
        img.scaledImage = img.histImg;
    } else {
        double scale = (img.scale > 0) ? img.scale : std::fabs(1 / ((double)(img.scale)));
        int scaledWidth = std::floor(img.histImg.width() * scale);
        int scaledHeight = std::floor(img.histImg.height() * scale);
        img.scaledImage = img.histImg.scaled(scaledWidth, scaledHeight);
    }
    setImageToWindow(img, "", window);
    if (img.scale == maxScale){
        ui -> bIncrease -> setEnabled(false);
    }
    ui -> bDecrease -> setEnabled(true);
    if (img.scale > 0){
        ui -> lbScale -> setText("Текущий масштаб: " + QString("%1 : 1").arg(img.scale));
    } else {
        ui -> lbScale -> setText("Текущий масштаб: " + QString("1 : %1").arg(std::fabs(img.scale)));
    }

}

//уменьшение масштаба
void MainWindow::on_bDecrease_clicked()
{
    GeoImage img(progress);
    if (!isActivateWindow()) {
        QMessageBox::critical(this, "Ошибка", "Выберите окно!");
        return;
    }

    QMdiSubWindow* window = ui->mdiArea->activeSubWindow();
    if (!getGeoImage(img, window)){
        QMessageBox::critical(this, "Ошибка", "Невозможно получить изображение из окна!");
        return;
    }

    if (img.scale - 1 < -maxScale){
        return;
    }

    img.scale -= 1;
    img.scale = img.scale == 0 ? -2 : img.scale;
    if (std::fabs(img.scale) == 1) {
        img.scaledImage = img.histImg;
    } else {
        double scale = (img.scale > 0) ? img.scale : std::fabs(1 / ((double)(img.scale)));
        int scaledWidth = std::floor(img.histImg.width() * scale);
        int scaledHeight = std::floor(img.histImg.height() * scale);
        img.scaledImage = img.histImg.scaled(scaledWidth, scaledHeight);
    }
    setImageToWindow(img, "", window);
    if (img.scale == -maxScale){
        ui -> bDecrease -> setEnabled(false);
    }
    ui -> bIncrease -> setEnabled(true);
    if (img.scale > 0){
        ui -> lbScale -> setText("Текущий масштаб: " + QString("%1 : 1").arg(img.scale));
    } else {
        ui -> lbScale -> setText("Текущий масштаб: " + QString("1 : %1").arg(std::fabs(img.scale)));
    }

}

//вывод масштаба изображения при выборе окна
void MainWindow::on_mdiArea_subWindowActivated(QMdiSubWindow *arg1)
{
    if (!arg1){
        return;
    }

    GeoImage img(progress);
    if (!getGeoImage(img, arg1)){
        QMessageBox::critical(this, "Ошибка", "Невозможно получить изображение из окна!");
        return;
    }

    if (img.scale > 0){
        ui -> lbScale -> setText("Текущий масштаб: " + QString("%1 : 1").arg(img.scale));
    } else {
        ui -> lbScale -> setText("Текущий масштаб: " + QString("1 : %1").arg(std::fabs(img.scale)));
    }

    ui -> bIncrease -> setEnabled(true);
    ui -> bDecrease -> setEnabled(true);
    if (img.scale == 4){
        ui -> bIncrease -> setEnabled(false);
    } else if (img.scale == -4){
        ui -> bDecrease -> setEnabled(false);
    }
}

//открытие изображения
void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QCoreApplication::applicationDirPath(), tr("Images (*.tiff *.tif)"));
    if (fileName != ""){
        GeoImage img(progress);
        openImg(img, fileName);
        ui -> lbScale -> setText("Текущий масштаб: 1 : 1");
    }
}

//сохранение изображения
void MainWindow::on_actionSaveRPC_triggered()
{
    if (ui -> mdiArea -> activeSubWindow() == Q_NULLPTR){
        QMessageBox::critical(this, "Ошибка", "Изображение не было выбрано!");
        return;
    }

    GeoImage img(progress);
    QMdiSubWindow* window = ui->mdiArea->activeSubWindow();
    if (!getGeoImage(img, window)){
        QMessageBox::critical(this, "Ошибка", "Невозможно получить изображение!");
        return;
    }

    if (img.rpc.getIsEmpty()){
        QMessageBox::critical(this, "Ошибка","Изображение не содержит RPC информацию!");
    } else {
        writeRPC(img.rpc);
        QMessageBox::information(this, "Успех", "Файл был записан");
    }

}

//оценка точности
void MainWindow::on_actionAccuracy_triggered()
{
    QMdiSubWindow* window = ui->mdiArea->activeSubWindow();
    if (!window) {
        QMessageBox::critical(this, "Ошибка", "Выберите изображение!");
        return;
    }

    GeoImage img(progress);
    if (!getGeoImage(img, window) || img.rpc.getIsEmpty()){
        QMessageBox::critical(this, "Ошибка", "Невозможно получить данные!");
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QCoreApplication::applicationDirPath(), tr("Text (*.txt)"));
    QByteArray ba = fileName.toLocal8Bit();
    const char *inFileName = ba.data();
    std::ifstream in(inFileName);
    if (!in.is_open()) {
        QMessageBox::critical(this, "Ошибка", "Файл не был открыт");
        return;
    }

    int n = 0;
    in >> n;
    if (n == 0){
        QMessageBox::critical(this, "Ошибка", "Файл не был открыт");
        return;
    }
    QVector<double> xVec(n, 0);
    QVector<double> yVec(n, 0);
    QVector<double> BVec(n, 0); //эталонные B, L, H (отм)
    QVector<double> LVec(n, 0);
    QVector<double> HVec(n, 0);
    for (int i = 0; i < n; i++){
        in >> xVec[i] >> yVec[i] >> BVec[i] >> LVec[i] >> HVec[i];
    }
    in.close();

    QVector<double> BCounted(n, 0); //посчитанные программой B и L (изм)
    QVector<double> LCounted(n, 0);
    Matrix<double> XYZ(n, 3);   //P
    Matrix<double> XYZL(n, 3);  //PL
    Matrix<double> XYZB(n, 3);  //PB
    QVector<double> RL(n, 0);
    QVector<double> RB(n, 0);

    for (int i = 0; i < n; i++){
        int k = 0;
        img.rpc.planar2geo(xVec[i], yVec[i], HVec[i], BCounted[i], LCounted[i], k);
        double B, L;
        if (!calcBL(B, L, BVec[i], LVec[i])){
            QMessageBox::critical(this, "Error", "B или L вне диапазона [-180; 180]");
            return;
        }
        calcP(XYZ, BVec[i], LVec[i], HVec[i], B, L, i);
        if (!calcBL(B, L, BVec[i], LCounted[i])){
            QMessageBox::critical(this, "Error", "B или L вне диапазона [-180; 180]");
            return;
        }
        calcP(XYZL, BVec[i], LCounted[i], HVec[i], B, L, i);
        if (!calcBL(B, L, BCounted[i], LVec[i])){
            QMessageBox::critical(this, "Error", "B или L вне диапазона [-180; 180]");
            return;
        }
        calcP(XYZB, BCounted[i], LVec[i], HVec[i], B, L, i);

        RL[i] = calcR(XYZ, XYZL, i);
        RB[i] = calcR(XYZ, XYZB, i);
//                std::cout << "; B = " << std::fixed << std::setprecision(9) << BVec[i] << "; L = " << LVec[i] <<
//                             "; X = " << XYZ[i][0]  << "; Y = " << XYZ[i][1] << "; Z = " << XYZ[i][2] <<
//                             "; XL = " << XYZL[i][0]  << "; YL = " << XYZL[i][1] << "; ZL = " << XYZL[i][2] <<
//                             "; XB = " << XYZB[i][0]  << "; YB = " << XYZB[i][1] << "; ZB = " << XYZB[i][2] <<
//                             "; RL = " << RL[i] << "; RB = " << RB[i] << std::endl;
    }

    double RMSl = calcRMS(RL);
    double RMSb = calcRMS(RB);
    double RMS = sqrt(RMSl * RMSl + RMSb * RMSb);

    AccuracyTable dialog(xVec, yVec, BCounted, LCounted, BVec, LVec, HVec, RL, RB, RMSb, RMSl, RMS, window->windowTitle(), fileName);
    dialog.exec();

}

//наложение двух изображений
void MainWindow::on_action2Images_triggered()
{
    QList<QMdiSubWindow*> subWindowList = ui->mdiArea->subWindowList();
    QStringList imagesList;
    for (int i = 0; i < subWindowList.length(); i++){
        imagesList.append(subWindowList[i]->windowTitle());
    }
    GeoImage img1, img2;
    TransformImagesDialog dialog(imagesList);
    int res = dialog.exec();
    if (res != QDialog::Accepted){
        return;
    }

    if (dialog.checkedImages.length() != 2 || dialog.checkedImages[0] == dialog.checkedImages[1]){
        QMessageBox::critical(this, "Ошибка", "Выберите два разных изображения!");
        dialog.checkedImages.clear();
        return;
    }

    for (int i = 0; i < subWindowList.length(); i++){
        if (subWindowList[i]->windowTitle() == dialog.checkedImages[0]){
            getGeoImage(img1, subWindowList[i]);
        } else if (subWindowList[i]->windowTitle() == dialog.checkedImages[1]){
            getGeoImage(img2, subWindowList[i]);
        }
    }

    if (img1.originMatrix.getColorsCount() != 1 || img2.originMatrix.getColorsCount() != 1){
        QMessageBox::critical(this, "Ошибка!", "Поддержка трехканальных изображений не осуществляется!");
        return;
    }
    transformImages(img1, img2, "1", "2", dialog.H);
//        transformImages(img2, img1, "2", "1");

}

//вывод гистограммы
void MainWindow::on_actionHistogram_triggered()
{
    QMdiSubWindow* window = ui->mdiArea->activeSubWindow();
    if (window == Q_NULLPTR){
        QMessageBox::critical(this, "Ошибка", "Выберите изображение");
        return;
    }
    GeoImage img(progress);
    if (!getGeoImage(img, window)){
        QMessageBox::critical(this, "Ошибка", "Невозможно получить изображение!");
    } else {
        Histogram hist(img);
        hist.exec();
    }
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionSave_triggered()
{
    QMdiSubWindow* window = ui->mdiArea->activeSubWindow();
    if (window){
        QMessageBox::critical(this, "Ошибка", "Выберите изображение!");
        return;
    }

    GeoImage img;
    if (getGeoImage(img, window)){
        QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить изображение"), QCoreApplication::applicationDirPath(), tr("Images (*.tif)"));
        img.save(fileName);
        QMessageBox::information(this, "Сохранение", "Файл сохранен!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Невозможно получить изображение!");
    }

}

void MainWindow::printToConsole(const QString& str){
//    QString var = QString::fromUtf8(str);
    std::cout << str.toLocal8Bit().data() << std::endl;
}
