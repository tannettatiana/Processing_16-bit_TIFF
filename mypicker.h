#ifndef MYPICKER_H
#define MYPICKER_H

#include "qwt_plot_picker.h"

class MyPicker : public QwtPlotPicker
{
public:
//    MyPicker();
    MyPicker(int xAxisId, int yAxisId, RubberBand rubberBand, DisplayMode trackerMode, QWidget *parent);
    QPointF transformCoordinates(const QPoint& pos);
};

#endif // MYPICKER_H
