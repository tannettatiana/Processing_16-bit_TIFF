#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QVector>

namespace Ui {
class Dialog;
}

class AccuracyTable : public QDialog
{
    Q_OBJECT

public:
    AccuracyTable(QVector<double>& x, QVector<double>& y, QVector<double>& BCount, QVector<double>& LCount,
                    QVector<double>& BOrig, QVector<double>& LOrig, QVector<double>& HOrig, QVector<double>& RL,
                    QVector<double>& RB, double RMSB, double RMSL, double RMS, const QString& imgName, const QString& fileName, QWidget *parent = 0);
    explicit AccuracyTable(QWidget *parent = 0);
    ~AccuracyTable();

    QVector<double> x;
    QVector<double> y;
    QVector<double> BCount;
    QVector<double> LCount;
    QVector<double> BOrig;
    QVector<double> LOrig;
    QVector<double> HOrig;
    QVector<double> RL;
    QVector<double> RB;
    double RMSB;
    double RMSL;
    double RMS;

private:
    Ui::Dialog *ui;
    void addItem(QString str, int i, int j);
};

#endif // DIALOG_H
