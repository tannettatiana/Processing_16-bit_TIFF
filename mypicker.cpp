#include "mypicker.h"

MyPicker::MyPicker(int xAxisId, int yAxisId, RubberBand rubberBand, DisplayMode trackerMode, QWidget *parent)
    : QwtPlotPicker(xAxisId, yAxisId, rubberBand, trackerMode, parent){

}

QPointF MyPicker::transformCoordinates(const QPoint& pos){
    return this->invTransform(pos);
}

