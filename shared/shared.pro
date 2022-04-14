#-------------------------------------------------
#
# Project created by QtCreator 2019-04-20T23:00:36
#
#-------------------------------------------------


QT       -= gui
QT       += sql concurrent

TARGET = shared
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++17

INCLUDEPATH += $$PWD/../sandbox/exile.h/
INCLUDEPATH += /usr/include/poppler/qt5/ /usr/include/quazip5

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += sqlitesearch.cpp \
    concurrentqueue.cpp \
    databasefactory.cpp \
    dbmigrator.cpp \
    defaulttextprocessor.cpp \
    dirscanworker.cpp \
    encodingdetector.cpp \
    filesaver.cpp \
    filescanworker.cpp \
    indexer.cpp \
    logger.cpp \
    looqsgeneralexception.cpp \
    common.cpp \
    looqsquery.cpp \
    nothingprocessor.cpp \
    odsprocessor.cpp \
    odtprocessor.cpp \
    pagedata.cpp \
    paralleldirscanner.cpp \
    pdfprocessor.cpp \
    processor.cpp \
    sandboxedprocessor.cpp \
    sqlitedbservice.cpp \
    tagstripperprocessor.cpp \
    utils.cpp \
    ../submodules/exile.h/exile.c

HEADERS += sqlitesearch.h \
    concurrentqueue.h \
    databasefactory.h \
    dbmigrator.h \
    defaulttextprocessor.h \
    dirscanworker.h \
    encodingdetector.h \
    filedata.h \
    filesaver.h \
    filescanworker.h \
    indexer.h \
    logger.h \
    looqsgeneralexception.h \
    looqsquery.h \
    nothingprocessor.h \
    odsprocessor.h \
    odtprocessor.h \
    pagedata.h \
    paralleldirscanner.h \
    pdfprocessor.h \
    processor.h \
    sandboxedprocessor.h \
    searchresult.h \
    sqlitedbservice.h \
    tagstripperprocessor.h \
    token.h \
    common.h \
    utils.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
RESOURCES = migrations/migrations.qrc
