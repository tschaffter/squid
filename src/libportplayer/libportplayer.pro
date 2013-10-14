# -------------------------------------------------
# Project created by QtCreator 2010-03-02T17:59:59
# -------------------------------------------------
#
# Build libportplayer.so
#
TARGET = portplayer
TEMPLATE = lib
VERSION = 1.0.10

CONFIG += link_pkgconfig
PKGCONFIG += dbus-1
INCLUDEPATH += ../utility
DEPENDPATH += ../utility

include(../utility/utility.pri)
include(libportplayer.pri)

LIBS += -lboost_filesystem \
    -lboost_program_options \
    -lglog

QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
