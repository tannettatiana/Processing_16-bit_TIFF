#ifndef GEOIMAGE_H
#define GEOIMAGE_H

#include "rastermatrix.h"
#include "rpc.h"
#include <QLabel>
#include <QImage>
#include <tiffio.h>
#include <QString>
#include "progress.h"
#include <iomanip>
#include <iostream>

const int depth = 10;
const int brightness_range = std::pow(2, depth);
const int max_brightness = brightness_range - 1;

class GeoImage : public QImage
{

public:
    GeoImage(Progress* progress);
    GeoImage();
    ~GeoImage();
    int load(const QString &fileName);
    int loadFromMatr(RasterMatrix<uint16_t> &matr);
    bool save(const QString &fileName);
    void setScaledImg(QImage& img);
    void printToConsole(const QString& str);

    std::vector<int> x1;
    std::vector<int> x2;

    QImage histImg;
    QImage scaledImage;
    double scale = 1;
    Rpc rpc;
    RasterMatrix<uint16_t> originMatrix;
    Progress* progress;

    QVector<uint32_t> histR;
    QVector<uint32_t> histG;
    QVector<uint32_t> histB;

    QString name;

    QString brightnessErr = "Ошибка в открытии изображения!";

private:
    template <typename T>
    int openAsMartixAndFindParameters(RasterMatrix<T>& originMatrix, TIFF* tif, uint16_t colorsCount, QImage& histImg);
    template <typename T>
    int findParameters(RasterMatrix<T>& originMatrix, uint16_t colorsCount, QImage& histImg);
    template <typename T>
    void contrasting(RasterMatrix<T>& originMatrix, QImage& histImg,  std::vector<int>& x1, std::vector<int>& x2);
};

#endif // GEOIMAGE_H
