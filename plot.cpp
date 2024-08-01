#include "plot.h"
#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_multi_barchart.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <QCheckBox>
#include <iostream>
#include <QMouseEvent>
#include <algorithm>

Plot::Plot(GeoImage& img, QLabel* labelHint, QLabel* labelx1x2, bool isGray, QCheckBox* cbZeroItems, QCheckBox* cbBoundaryZeroItems, QWidget *parent ):
   histR(img.histR), histG(img.histG), histB(img.histB), x1(img.x1), x2(img.x2), QwtPlot( parent )
{
    this->cbBoundaryZeroItems = cbBoundaryZeroItems;
    this->cbZeroItems = cbZeroItems;
    curve1 = new QwtPlotCurve();
    curve2 = new QwtPlotCurve();
    curve3 = new QwtPlotCurve();
    curve1->attach( this );
    curve2->attach( this );
    curve3->attach( this );

    this->labelHint = labelHint;
    this->labelx1x2 = labelx1x2;

    setAutoFillBackground( true );

    setPalette( Qt::white );
    canvas()->setPalette( QColor( "White" ) );

    setTitle( "Гистограмма" );

    d_barChartItem = new BarChartPlot( "Bar Chart " );
    d_barChartItem->cbZeroItem = cbZeroItems;

    d_barChartItem->setLayoutPolicy(QwtPlotAbstractBarChart::ScaleSamplesToAxes);
    d_barChartItem->setSpacing( 0 );
    d_barChartItem->setMargin( 0 );

    d_barChartItem->attach( this );

    //insertLegend( new QwtLegend() );
    d_barChartItem->isGray = isGray;
    if (isGray){
        d_barChartItem->channel = gray;
    } else {
        d_barChartItem->channel = red;
    }

    populate(histR);
    configureBarChart();
    setPlot(0);

    setAutoReplot(true);
}

//построение графика контрастирования
void Plot::setPlot(int channel){
    QwtScaleMap sm = this->canvasMap(QwtPlot::yLeft);
    QwtScaleMap sm2 = this->canvasMap(QwtPlot::xBottom);
    insertLine(Qt::black, sm2.s1(), 0, x1[channel], curve1);
    insertLine(Qt::black, sm.s2(), x2[channel], sm2.s2(), curve3);
    insertDiagonal(Qt::black, x1[channel], 0, x2[channel], sm.s2(), curve2);
}

//задание значений гистограммы
void Plot::populate(QVector<uint32_t>& hist)
{
    QVector<uint32_t> barValues;
    if (cbBoundaryZeroItems->isChecked() && cbZeroItems->isChecked()){
        auto firstNonZeroIt = std::find_if(hist.begin() + 1, hist.end(), [](uint32_t value) { return value > 0; });
        auto lastNonZeroIt = std::find_if(std::reverse_iterator<QVector<uint32_t>::iterator>(hist.end()),
                                          std::reverse_iterator<QVector<uint32_t>::iterator>(hist.begin() + 1),
                                          [](uint32_t value) { return value > 0; });
        auto lastNonZeroBaseIt = lastNonZeroIt.base();
        std::copy(hist.begin() + 1, lastNonZeroBaseIt, std::back_inserter(barValues));
        int start = firstNonZeroIt - hist.begin();
        int end = hist.end() - lastNonZeroBaseIt;
        end = hist.size() - end - 1;
        setAxisScale( QwtPlot::xBottom, start, end);
    } else if (cbBoundaryZeroItems->isChecked()){
        auto firstNonZeroIt = std::find_if(hist.begin(), hist.end(), [](uint32_t value) { return value > 0; });
        auto lastNonZeroIt = std::find_if(std::reverse_iterator<QVector<uint32_t>::iterator>(hist.end()),
                                          std::reverse_iterator<QVector<uint32_t>::iterator>(hist.begin()),
                                          [](uint32_t value) { return value > 0; });
        auto lastNonZeroBaseIt = lastNonZeroIt.base();
        std::copy(hist.begin(), lastNonZeroBaseIt, std::back_inserter(barValues));
        int start = firstNonZeroIt - hist.begin();
        int end = hist.end() - lastNonZeroBaseIt;
        end = hist.size() - end - 1;
        setAxisScale( QwtPlot::xBottom, start, end);
//        d_barChartItem->style = QwtColumnSymbol::Plain;
    } else if (cbZeroItems->isChecked()){
        barValues.resize(hist.size() - 1);
        std::copy(hist.begin() + 1, hist.end(), barValues.data());
    }
    else {
//        barValues.resize(hist.size());
//        std::copy(hist.begin(), hist.end(), barValues.data());
        barValues = hist;
        setAxisScale( QwtPlot::xBottom, 0, hist.size());
//        d_barChartItem->style = QwtColumnSymbol::NoFrame;
    }
    const int numSamples = barValues.size();

//    QwtColumnSymbol *symbol = new QwtColumnSymbol( QwtColumnSymbol::Box );
//    symbol->setLineWidth( 1 );
//    symbol->setFrameStyle( QwtColumnSymbol::NoFrame );
//    QString color;
//    if (channel == blue){
//        color = "Blue";
//    } else if (channel == red){
//        color = "Red";
//    } else if (channel == green){
//        color = "Green";
//    }
//    if (isGray){
//        color = "Gray";
//    }
//    symbol->setPalette( QPalette( color ) );
//    d_barChartItem->setSymbol(  );

    d_barChartItem->setLegendMode(QwtPlotBarChart::LegendBarTitles);
    for ( int i = 0; i < numSamples; i++ )
    {
        d_barChartItem->barTitle(i) = QwtText("");
    }

    QVector<double> series(numSamples, 0);
    for ( int i = 0; i < numSamples; i++ )
    {
        series[i] = (double)barValues[i];
    }

    d_barChartItem->setSamples(series);
    setAxisScale( QwtPlot::yLeft, 0, *std::max_element(series.begin(), series.end()));
}

//установка режима графика
void Plot::setMode( int mode )
{
    if ( mode == 0 ){
        d_barChartItem->channel = red;
        currentChannel = red;
        populate(histR);
        labelx1x2->setText(QString("x' = %1").arg(x1[0]) + QString("; x'' = %1").arg(x2[0]));
        setPlot(0);
    } else if (mode == 1){
        currentChannel = green;
        d_barChartItem->channel = green;
        populate(histG);
        labelx1x2->setText(QString("x' = %1").arg(x1[1]) + QString("; x'' = %1").arg(x2[1]));
        setPlot(1);
    } else {
        currentChannel = blue;
        d_barChartItem->channel = blue;
        populate(histB);
        labelx1x2->setText(QString("x' = %1").arg(x1[2]) + QString("; x'' = %1").arg(x2[2]));
        setPlot(2);
    }
    onSelected(previousPos);
}

//вывод значений диаграммы
void Plot::onSelected(const QPointF& p){
    int x = p.x();
    if (!(x >= 0 && x <= max_brightness)){
        return;
    }

    int y;
    if (d_barChartItem->channel == red){
        y = histR[x];
    } else if (d_barChartItem->channel == green){
        y = histG[x];
    } else if (d_barChartItem->channel == blue){
        y = histB[x];
    }
    labelHint->setText(QString("bright: %1").arg(x) + QString("; number: %1").arg(y));
    previousPos = p;

}

//настройка графика
void Plot::configureBarChart()
{
    QwtPlot::Axis axis1, axis2;

    axis1 = QwtPlot::xBottom;
    axis2 = QwtPlot::yLeft;

    d_barChartItem->setOrientation( Qt::Vertical );

    setAxisScale( axis1, 0, d_barChartItem->dataSize() - 1, 100 );
    setAxisAutoScale( axis2 );

    QwtScaleDraw *scaleDraw1 = axisScaleDraw( axis1 );
    scaleDraw1->enableComponent( QwtScaleDraw::Backbone, true );
    scaleDraw1->enableComponent( QwtScaleDraw::Ticks, true );

    QwtScaleDraw *scaleDraw2 = axisScaleDraw( axis2 );
    scaleDraw2->enableComponent( QwtScaleDraw::Backbone, true );
    scaleDraw2->enableComponent( QwtScaleDraw::Ticks, true );

    plotLayout()->setAlignCanvasToScale( axis1, true );
    plotLayout()->setAlignCanvasToScale( axis2, false );

    plotLayout()->setCanvasMargin( 0 );
    updateCanvasMargins();

    replot();
}

//вставка прямой линии
void Plot::insertLine( const QColor &c, double base, int start, int end, QwtPlotCurve *curve)
{
    curve->setPen( c, 3 );

    double x[2];
    double y[sizeof( x ) / sizeof( x[0] )];

    x[0] = start;
    y[0] = base;
    x[1] = end;
    y[1] = base;

    curve->setSamples( x, y, sizeof( x ) / sizeof( x[0] ) );
}

//вставка диагональной линии
void Plot::insertDiagonal(const QColor &c, int startX, int startY, int endX, int endY, QwtPlotCurve *curve){
    curve->setPen( c, 3 );
    double x[2];
    double y[sizeof( x ) / sizeof( x[0] )];
    x[0] = startX;
    y[0] = startY;
    x[1] = endX;
    y[1] = endY;
    curve->setSamples( x, y, sizeof( x ) / sizeof( x[0] ) );
}

void Plot::mouseMoveEvent(QMouseEvent *event){
//    int screenX = event->pos.x();
//    double x = this->invTransform(QwtPlot::xBottom, event->pos().x());
//    double y = this->invTransform(QwtPlot::yLeft, event->pos().y());
    QPoint pos;
    pos.setX(picker->trackerPosition().x());
    pos.setY(picker->trackerPosition().y());
    QPointF newPos = picker->transformCoordinates(pos);
    onSelected(newPos);
}
