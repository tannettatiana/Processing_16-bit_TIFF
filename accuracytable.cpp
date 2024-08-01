#include "accuracytable.h"
#include "ui_accuracytable.h"

AccuracyTable::AccuracyTable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

//добавление элемента
void AccuracyTable::addItem(QString str, int i, int j){
    QTableWidgetItem *item = new QTableWidgetItem(str);
    ui->table->setItem(i, j, item);
}

AccuracyTable::AccuracyTable(QVector<double>& x, QVector<double>& y, QVector<double>& BCounted, QVector<double>& LCounted,
               QVector<double>& BOrig, QVector<double>& LOrig, QVector<double>& HOrig, QVector<double>& RL,
               QVector<double>& RB, double RMSb, double RMSl, double RMS, const QString& imgName, const QString& fileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Оценка точности изображения " + imgName);
    this->x = x;
    this->y = y;
    this->BCount = BCounted;
    this->LCount = LCounted;
    this->BOrig = BOrig;
    this->LOrig = LOrig;
    this->HOrig = HOrig;
    this->RL = RL;
    this->RB = RB;
    this->RMSL = RMSl;
    this->RMSB = RMSb;
    this->RMS = RMS;

    int prec = 8;

    for (int i = 0; i < x.length(); i++){
        ui->table->insertRow(ui->table->rowCount());
        addItem(QString("%1").arg(i + 1, 1, 'd', 0, '0'), i, 0);
        addItem(QString("%1").arg(x[i], 5, 'f', 1, '0'), i, 1);
        addItem(QString("%1").arg(y[i], 5, 'f', 1, '0'), i, 2);
        addItem(QString("%1").arg(BCounted[i], 5, 'f', prec, '0'), i, 3);
        addItem(QString("%1").arg(LCounted[i], 5, 'f', prec, '0'), i, 4);
        addItem(QString("%1").arg(BOrig[i], 5, 'f', prec, '0'), i, 5);
        addItem(QString("%1").arg(LOrig[i], 5, 'f', prec, '0'), i, 6);
        addItem(QString("%1").arg(HOrig[i], 5, 'f', 3, '0'), i, 7);
        addItem(QString("%1").arg(RL[i], 5, 'f', prec, '0'), i, 8);
        addItem(QString("%1").arg(RB[i], 5, 'f', prec, '0'), i, 9);
    }
    ui->table->verticalHeader()->setVisible(false);
    ui->table->resizeColumnsToContents();
    ui->table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->lRMSl->setText("RMSL = " + QString("%1 м").arg(RMSl, 5, 'f', prec, '0'));
    ui->lRMSb->setText("RMSB = " + QString("%1 м").arg(RMSb, 5, 'f', prec, '0'));
    ui->lRMS->setText("RMS = " + QString("%1 м").arg(RMS, 5, 'f', prec, '0'));
    ui->lFile->setText("Файл с точками: " + fileName);
}

AccuracyTable::~AccuracyTable()
{
    delete ui;
}
