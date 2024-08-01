#ifndef PROGRESS_H
#define PROGRESS_H

#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <qmath.h>
#include <QException>
#include <QMessageBox>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdexcept>

class Progress : public QWidget
{
    Q_OBJECT
public:
    explicit Progress(QWidget *parent = 0);
    QProgressBar *progress;
    QLabel* lbProgressName;
    QLabel* lbProgressPercent;
    QPushButton* bStop;
    std::chrono::time_point<std::chrono::system_clock> timeStart;
    std::chrono::time_point<std::chrono::system_clock> timeEnd;
    std::chrono::duration<double> elapsed_seconds;

    bool wasProcessStoppedByUser = false; //остановка процесса пользователем
    bool wasProcessStopped = true; //остановка процесса в принципе

    void start(const QString& progressName);
    void stop(); //stop() нельзя вызывать после вызова bStop_pressed(), т.к. это приводит к исключению
    bool set_progress(const double percent);
    void printToConsole(const QString& str);

signals:

public slots:
    void bStop_pressed();

};

#endif // PROGRESS_H
