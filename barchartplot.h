#ifndef BARCHARTPLOT_H
#define BARCHARTPLOT_H

#include "qwt_plot_barchart.h"
#include <QCheckBox>

enum channels{
    red,
    green,
    blue,
    gray,
};

class BarChartPlot : public QwtPlotBarChart
{
public:
    BarChartPlot(const char* name);
    virtual QwtColumnSymbol* specialSymbol(int sampleIndex, const QPointF &sample ) const;

    channels channel; //текущий канал, для которого показывается гистограмма
    bool isGray = false; //является ли изображение чб
    QCheckBox* cbZeroItem;
//    int style;

};

#endif // BARCHARTPLOT_H
