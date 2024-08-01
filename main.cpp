#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

#ifdef NDEBUG
#warning NDEBUG
#else
    #warning DEBUG
#endif
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();

}
