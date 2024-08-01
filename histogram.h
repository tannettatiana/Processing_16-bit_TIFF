#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QDialog>
#include <QToolBar>
#include <QToolButton>
#include <QComboBox>
#include "plot.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QMenuBar>
#include "geoimage.h"
#include "mypicker.h"

namespace Ui {
class Histogram;
}

class Histogram : public QDialog
{
    Q_OBJECT

public:
    explicit Histogram(GeoImage& img, QWidget *parent = 0);
    ~Histogram();

protected:
    void mouseMoveEvent(QMouseEvent *event);       //событие перемещения мыши

private slots:
    void on_cbZeroItems_stateChanged(int arg1);

    void on_cbBoundaryZeroItems_stateChanged(int arg1);

private:
    Ui::Histogram *ui;
    QVBoxLayout *layout;
    Plot *d_chart;
    MyPicker *d_picker;
    //void onSelected(const QPointF&);
    QVector<uint32_t>& histR;
    QVector<uint32_t>& histG;
    QVector<uint32_t>& histB;

};

#endif // HISTOGRAM_H
