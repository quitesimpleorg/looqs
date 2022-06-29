#-------------------------------------------------
#
# Project created by QtCreator 2018-08-09T12:23:48
#
#-------------------------------------------------

QT       += core concurrent gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17



TARGET = looqs-gui
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutdialog.cpp \
    ipcpreviewclient.cpp \
    ipcpreviewworker.cpp \
    ipcserver.cpp \
        main.cpp \
        mainwindow.cpp \
      clicklabel.cpp \
    previewgenerator.cpp \
    previewgeneratormapfunctor.cpp \
    previewgeneratorpdf.cpp \
    previewgeneratorplaintext.cpp \
    previewresult.cpp \
    previewresultpdf.cpp \
    previewresultplaintext.cpp \
    renderconfig.cpp \
    rendertarget.cpp

HEADERS += \
    aboutdialog.h \
    ipc.h \
    ipcpreviewclient.h \
    ipcpreviewworker.h \
    ipcserver.h \
        mainwindow.h \
    clicklabel.h \
    previewgenerator.h \
    previewgeneratormapfunctor.h \
    previewgeneratorpdf.h \
    previewgeneratorplaintext.h \
    previewresult.h \
    previewresultpdf.h \
    previewresultplaintext.h \
    renderconfig.h \
    rendertarget.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += /usr/include/poppler/qt5/

QT += widgets sql

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../shared/release/ -lshared
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../shared/debug/ -lshared
else:unix: LIBS += -L$$OUT_PWD/../shared/ -lshared

LIBS += -luchardet -lpoppler-qt5 -lquazip5

packagesExist(quazip1-qt5) {
	PKGCONFIG += quazip1-qt5
	CONFIG += link_pkgconfig
	LIBS -= -lquazip5
}

INCLUDEPATH += $$PWD/../shared
DEPENDPATH += $$PWD/../shared

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../shared/release/libshared.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../shared/debug/libshared.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../shared/release/shared.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../shared/debug/shared.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../shared/libshared.a

RESOURCES = ../looqs.svg
