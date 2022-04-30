#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load(":lang/daspelling_da_DK");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
