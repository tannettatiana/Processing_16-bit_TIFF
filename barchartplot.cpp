#include "barchartplot.h"
#include <qwt_column_symbol.h>
#include <QPalette>


BarChartPlot::BarChartPlot(const char* name) : QwtPlotBarChart(name)
{

}

QwtColumnSymbol* BarChartPlot::specialSymbol(int sampleIndex, const QPointF &sample ) const
{
    QwtColumnSymbol* symbol = new QwtColumnSymbol( QwtColumnSymbol::Box );

    symbol->setLineWidth( 2 );
    symbol->setFrameStyle( QwtColumnSymbol::NoFrame );

    QString color;
    if (channel == blue){
        color = "Blue";
    } else if (channel == red){
        color = "Red";
    } else if (channel == green){
        color = "Green";
    }
    if (isGray){
        color = "Gray";
    }
    QString zeroColor = "White";
    const QColor c( sample.y() > 0 ? color : zeroColor );
//    const QColor c(color);
    symbol->setPalette( QPalette( c ) );

    return symbol;
}
