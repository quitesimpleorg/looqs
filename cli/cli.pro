QT -= gui
QT += sql concurrent
CONFIG += c++17 console
CONFIG -= app_bundle
TARGET = looqs

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES += \
    commandtag.cpp \
        main.cpp \
    commandadd.cpp \
    commanddelete.cpp \
    commandupdate.cpp \
    commandsearch.cpp \
    commandlist.cpp \
    command.cpp

HEADERS += \
    command.h \
    commandadd.h \
    commanddelete.h \
    commandtag.h \
    commandupdate.h \
    commandsearch.h \
    commandlist.h

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
