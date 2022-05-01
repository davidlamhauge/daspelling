#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSettings settings("TeamLamhauge", "daSpelling");
    QString mLang = settings.value("lang").toString();
    if (mLang == "")
    {
        mLang = ":lang/lang/daspelling_da_DK";
    }

    QTranslator translator;
    translator.load(mLang);
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
