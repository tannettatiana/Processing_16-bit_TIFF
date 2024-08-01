#include "mylabel.h"
#include <QMouseEvent>
#include <iostream>
#include <cmath>

MyLabel::MyLabel(GeoImage &contrastedImg, QWidget *parent) :
    QLabel(parent), contrastedImg(contrastedImg)
{
    this->setMouseTracking(true);
}

MyLabel::MyLabel(QWidget *parent) :
    QLabel(parent)
{
    this->setMouseTracking(true);
}

MyLabel::~MyLabel()
{
}

void MyLabel::setCoordinatesLabel(QLabel* coordinatesLabel, QLabel* brightsLabel, QLabel* geoCoordinates){
    coordinates = coordinatesLabel;
    brights = brightsLabel;
    geo = geoCoordinates;
}

GeoImage& MyLabel::getGeoImage(){
    return contrastedImg;
}

//void MyLabel::setImages(GeoImage &contrastedImg){

//    this -> contrastedImg = contrastedImg;
//}

//обработчик движения мыши
void MyLabel::mouseMoveEvent(QMouseEvent *event){
    int width = contrastedImg.histImg.width();
    int height = contrastedImg.histImg.height();
    QPoint cursorPos = event->pos();
    x = cursorPos.x() + 0.5;
    y = cursorPos.y() + 0.5;
    if (contrastedImg.scale > 0){
        x = x / ((double)contrastedImg.scale);
        y = y / ((double)contrastedImg.scale);
    } else {
        x *= std::fabs(contrastedImg.scale);
        y *= std::fabs(contrastedImg.scale);
    }
    if ((x < width) && (y < height)){
        coordinates->setText("Pix coordinates:\n" + QString("x = %1").arg(x, 5,'f', 3, '0') + QString(" y = %1").arg(y, 5,'f', 3, '0'));
        int xInt = std::floor(x);
        int yInt = std::floor(y);
        if (contrastedImg.originMatrix.getColorsCount() == 1){
            uint16_t bright = contrastedImg.originMatrix.getPixel(yInt, xInt);
            brights->setText("Brights: \n" + QString("g: %1").arg(bright));
        } else {
            uint16_t red = contrastedImg.originMatrix.getPixel(yInt, xInt, 0);
            uint16_t green = contrastedImg.originMatrix.getPixel(yInt, xInt, 1);
            uint16_t blue = contrastedImg.originMatrix.getPixel(yInt, xInt, 2);
            brights->setText("Brights: \n" + QString("r: %1").arg(red) + " " + QString("g: %1").arg(green) + " " + QString("b: %1").arg(blue));
        }
        double B = 0;
        double L = 0;
        int k = 0;
        if (contrastedImg.rpc.planar2geo(x, y, contrastedImg.rpc.data.HEIGHT_OFF, B, L, k)){
            geo->setText("Geo coordinates:\n" + QString("B = %1").arg(B, 5,'f', 6,'0') + QString(" L = %1").arg(L, 5,'f', 6,'0')/* + QString(" Iterations = %1").arg(k)*/);
        } else{
            geo->setText("Geo coordinates:\n" + QString("B = -") + QString(" L = -")/* + QString(" Iterations = -")*/);
        }
    } else {
        coordinates->setText("Pix coordinates:\n" + QString("x = -") + QString(" y = -"));
        brights->setText("Brights: \n" + QString("r: -") + " " + QString("g: -") + " " + QString("b: -"));
        geo->setText("Geo coordinates:\n" + QString("B = -") + QString(" L = -")/* + QString(" Iterations = -")*/);
    }
}

void MyLabel::setImage(GeoImage& img){
    contrastedImg = img;
}
