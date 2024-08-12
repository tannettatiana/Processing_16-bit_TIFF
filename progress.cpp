#include "progress.h"
#include <QCoreApplication>

Progress::Progress(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    this->setLayout(layout);


    lbProgressName = new QLabel("");
    lbProgressName->setMaximumWidth(1800);
    lbProgressName->setMinimumWidth(100);
    lbProgressName->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
//    layout->addStretch(1);
    layout->addWidget(lbProgressName);

    progress = new QProgressBar();
    progress->setMinimum(0);
    progress->setMaximum(100);
    progress->setValue(0);
    progress->setFixedWidth(300);
    progress->setTextVisible(false);
    progress->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    layout->addStretch(0);
    layout->addWidget(progress);

    lbProgressPercent = new QLabel("");
    lbProgressPercent->setFixedWidth(60);
    lbProgressPercent->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    layout->addStretch(0);
    layout->addWidget(lbProgressPercent);

    bStop = new QPushButton("Прервать");
    bStop->setFixedWidth(150);
    bStop->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    layout->addStretch(0);
    layout->addWidget(bStop);

    bStop->setEnabled(false);

    connect(bStop, SIGNAL(pressed()), this, SLOT(bStop_pressed()));
}

//нажатие кнопки остановки процесса
void Progress::bStop_pressed(){
    QMessageBox::information(this, "Информация", "Процесс был остановлен!");
    wasProcessStoppedByUser = true;
    wasProcessStopped = true;
    timeEnd = std::chrono::system_clock::now();
    elapsed_seconds = timeEnd - timeStart;
    printToConsole(QString(" Время обработки (от начала до остановки пользователем): %1 с").arg(elapsed_seconds.count(), 3, 'f', 3,'0'));
//    std::cout << std::fixed << std::setprecision(3) << " Processing time (from starting to stopping by user): " << elapsed_seconds.count() << " s" << std::endl;
    progress->setValue(0);
    lbProgressPercent->setText("");
    lbProgressName->setText("");
    bStop->setEnabled(false);
}

//старт процесса
void Progress::start(const QString& progressName){
    if (!wasProcessStopped){
        throw std::runtime_error("Failed starting!");
    } else {
        bStop->setEnabled(true);
        wasProcessStoppedByUser = false;
        wasProcessStopped = false;
        progress->reset();
        lbProgressName->setText(progressName);
        printToConsole(progressName);
        lbProgressPercent->setText("0.0%");
//        QCoreApplication::processEvents();
        timeStart = std::chrono::system_clock::now();
    }
}

//stop() нельзя вызывать после вызова bStop_pressed(), т.к. это приводит к исключению
//завершение процесса
void Progress::stop(){
    if (wasProcessStopped){
        throw std::runtime_error("Failed stopping!");
    } else {
        //set_progress(0);
        wasProcessStopped = true;
        timeEnd = std::chrono::system_clock::now();
        elapsed_seconds = timeEnd - timeStart;
        printToConsole(QString("Время обработки: %1 с").arg(elapsed_seconds.count(), 3, 'f', 3,'0'));
//        std::cout << std::fixed << std::setprecision(3) << " Processing time: " << elapsed_seconds.count() << " s" << std::endl;
        lbProgressName->setText("");
        progress->setValue(0);
        lbProgressPercent->setText("");
        bStop->setEnabled(false);
    }
}

//установка значения прогресса
bool Progress::set_progress(const double percent){
    if (!(std::isnan(percent) || percent > 100 || percent < 0 || percent < progress->value())){
        progress->setValue(qRound(percent));
        lbProgressPercent->setText(QString("%1%").arg(percent, 2,'f', 2,'0'));
        printToConsole(QString("Прогресс: %1%").arg(percent, 2,'f', 1,'0'));
//        std::cout << "\r Progress: " << percent << "%";
//        std::cout.flush();
        //QCoreApplication::processEvents();
        return wasProcessStoppedByUser;
    }
}

void Progress::printToConsole(const QString& str){
//    QString var = QString::fromUtf8(str);
    std::cout << str.toLocal8Bit().data() << std::endl;
}
