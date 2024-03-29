QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 embed_translations

TARGET = daspelling
QMAKE_APPLICATION_BUNDLE_NAME = Daspelling

VERSION = 0.9.13
DEFINES += APP_VERSION=\"\\\"$${VERSION}\\\"\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    preferencemanager.cpp \
    recordwords.cpp

HEADERS += \
    mainwindow.h \
    preferencemanager.h \
    recordwords.h

FORMS += \
    mainwindow.ui \
    preferencemanager.ui \
    recordwords.ui

TRANSLATIONS += \
    lang/daspelling_da_DK.ts \
    lang/daspelling_en.ts \
    lang/daspelling_de.ts \
    lang/daspelling_fo.ts

# Default rules for deployment.
# qnx: target.path = /tmp/$${TARGET}/bin
# else: unix:!android: target.path = /opt/$${TARGET}/bin
# !isEmpty(target.path): INSTALLS += target

DISTFILES += \
    lang/daspelling_fo.qm \
    lang/daspelling_fo.ts

RESOURCES += \
    icons.qrc

RC_ICONS += icons/spelling.ico

unix:!macx {
    target.path = $${PREFIX}

    desktopentry.files = icons/daspelling.desktop
    desktopentry.path = $${PREFIX}/share/applications

    icon.files = icons/spelling.png
    icon.path = $${PREFIX}/share/icons/hicolor/256x256/apps

    INSTALLS += target desktopentry icon
}
