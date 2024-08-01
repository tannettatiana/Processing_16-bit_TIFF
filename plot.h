#ifndef _BAR_CHART_H_

#include <qwt_plot.h>
#include <vector>
#include <qwt_plot_barchart.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include "geoimage.h"
#include <qwt_plot_curve.h>
#include <QCheckBox>
#include "mypicker.h"
#include "barchartplot.h"

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(GeoImage& img, QLabel* labelHint, QLabel* labelx1x2, bool isGray, QCheckBox* cbZeroItems, QCheckBox* cbBoundaryZeroItems, QWidget * = 0 );
    MyPicker* picker; //объект для определения координат под курсором
    QCheckBox* cbBoundaryZeroItems;
    QCheckBox* cbZeroItems;
    int currentChannel;

public Q_SLOTS:
    void setMode( int );
    void configureBarChart();
    void onSelected(const QPointF&);    

private:
    void mouseMoveEvent(QMouseEvent *event);

private:
    void populate(QVector<uint32_t>& hist);
    void setPlot(int channel);
    void insertLine( const QColor &c, double base, int start, int end, QwtPlotCurve *curve);
    void insertDiagonal(const QColor &c, int startX, int startY, int endX, int endY, QwtPlotCurve *curve);

    QwtPlotCurve *curve1; //линия до нижней границы контрастирования
    QwtPlotCurve *curve2; //линия от нижней до верхней границы контрастирвоания
    QwtPlotCurve *curve3; //линия от верхней границы контрастирования

    BarChartPlot *d_barChartItem; //столбчатая диаграмма

    QVector<uint32_t>& histR; //гистограмма красного (серого в случае чб) канала
    QVector<uint32_t>& histG; //гистограмма зеленого канала
    QVector<uint32_t>& histB; //гистограмма синего канала

    std::vector<int>& x1; //нижняя граница контрастирования
    std::vector<int>& x2; //верхняя граница контрастирования

//    channels channel; //текущий канал, для которого показывается гистограмма
//    bool isGray = false; //является ли изображение чб

    QLabel* labelHint; //информация о выбранном столбце гистограммы
    QLabel* labelx1x2; //границы контрастирования

    QPointF previousPos; //предыдущая позиция курсора (используется при переключении канала для вывода корректных значений)


};

#endif
