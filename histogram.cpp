#include "histogram.h"
#include "ui_histogram.h"

Histogram::Histogram(GeoImage& img, QWidget *parent) :
    histR(img.histR), histG(img.histG), histB(img.histB), QDialog(parent),
    ui(new Ui::Histogram)
{
    ui->setupUi(this);

    this->setWindowTitle("Гистограмма " + img.name);

    if (img.originMatrix.getColorsCount() == 1){
        d_chart = new Plot(img, ui->label, ui->x1_x2, true, ui->cbZeroItems, ui->cbBoundaryZeroItems);
    } else {
        d_chart = new Plot(img, ui->label, ui->x1_x2, false, ui->cbZeroItems, ui->cbBoundaryZeroItems);
    }
    layout = new QVBoxLayout();
    layout->addWidget(d_chart);
    ui->widget->setLayout(layout);

    QComboBox *typeBox = ui->comboBox;

    typeBox->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    if (img.originMatrix.getColorsCount() == 1){
        typeBox->setEnabled(false);
        typeBox->addItem( "Серый" );
    } else {
        typeBox->addItem( "Красный" );
        typeBox->addItem( "Зеленый" );
        typeBox->addItem( "Синий" );
    }


    d_chart->setMode( typeBox->currentIndex() );
    connect(typeBox, SIGNAL(currentIndexChanged(int)), d_chart, SLOT(setMode(int)));

    d_picker = new MyPicker( QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, d_chart->canvas() );
    d_picker->setStateMachine(new QwtPickerDragPointMachine());
    d_picker->setRubberBandPen( QColor( Qt::green ) );
    d_picker->setRubberBand( QwtPicker::CrossRubberBand );
    d_picker->setTrackerPen( QColor( Qt::black ) );

    connect(d_picker, SIGNAL(selected(const QPointF&)), d_chart, SLOT(onSelected(const QPointF&)));
    connect(d_picker, SIGNAL(moved(const QPointF&)), d_chart, SLOT(onSelected(const QPointF&)));
    connect(d_picker, SIGNAL(appended(const QPointF&)), d_chart, SLOT(onSelected(const QPointF&)));

    setMouseTracking(true);
    d_chart->setMouseTracking(true);

    d_chart->picker = d_picker;
}

void Histogram::mouseMoveEvent(QMouseEvent *event){
}

Histogram::~Histogram()
{
    delete ui;
}

void Histogram::on_cbZeroItems_stateChanged(int arg1)
{
    d_chart->setMode(d_chart->currentChannel);
//    d_chart->replot();
}

void Histogram::on_cbBoundaryZeroItems_stateChanged(int arg1)
{
    d_chart->setMode(d_chart->currentChannel);
}
