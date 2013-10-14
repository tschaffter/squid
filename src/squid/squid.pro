# -------------------------------------------------
# Project created by QtCreator 2010-03-02T17:59:59
# -------------------------------------------------
TARGET = squid
TEMPLATE = app
VERSION = 1.0.10

CONFIG += link_pkgconfig
PKGCONFIG += dbus-1
INCLUDEPATH += ../utility \
    ../libportplayer \
    ../qportplayer \
    ../libsquid
DEPENDPATH += ../utility \
    ../libportplayer \
    ../qportplayer \
    ../libsquid

include(../../lib/qtsingleapplication-2.6_1-opensource/src/qtsingleapplication.pri)
include(../utility/utility.pri)
include(../qportplayer/qportplayer.pri)
include(squid.pri)

SOURCES += main.cpp
LIBS += -ldc1394 \
    -lraw1394 \
    -lboost_system \
    -lboost_filesystem \
    -lboost_program_options \
    -lglog \
    -lqledplugin \
    -L../libportplayer \
    -lportplayer \
    -L../libsquid \
    -lsquid \
    -ltiff

QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
