# -------------------------------------------------
# Project created by QtCreator 2010-03-02T17:59:59
# -------------------------------------------------
TARGET = qportplayer
TEMPLATE = app
VERSION = 1.0.10

CONFIG += link_pkgconfig
PKGCONFIG += dbus-1
INCLUDEPATH += ../utility \
    ../libportplayer
DEPENDPATH += ../utility \
    ../libportplayer

include(../utility/utility.pri)
include(qportplayer.pri)

SOURCES += main.cpp
LIBS += -lboost_filesystem \
    -lboost_system \
    -lboost_program_options \
    -lglog \
    -lqledplugin \
    -L../libportplayer \
    -lportplayer

QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

HEADERS +=












