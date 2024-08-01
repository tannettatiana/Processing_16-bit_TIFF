#ifndef MYLABEL_H
#define MYLABEL_H

#include "geoimage.h"
#include <QLabel>
#include <QImage>

class MyLabel : public QLabel
{
    Q_OBJECT

public:
    explicit MyLabel(GeoImage &contrastedImg, QWidget *parent = 0);
    explicit MyLabel(QWidget *parent = 0);
    ~MyLabel();
    void setCoordinatesLabel(QLabel* coordinatesLabel, QLabel* brightsLabel, QLabel* geoCoordinates);
    GeoImage& getGeoImage();
    void setImage(GeoImage& img);

    QLabel* coordinates;
    QLabel* brights;
    QLabel* geo;

    GeoImage contrastedImg;
    double x;
    double y;

private slots:

    void mouseMoveEvent(QMouseEvent* event);

private:

};

#endif // MYLABEL_H
