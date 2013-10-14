# -------------------------------------------------
# Project created by QtCreator 2010-03-02T17:59:59
# -------------------------------------------------
# Build libsquid.so
TARGET = squid
TEMPLATE = lib
VERSION = 1.0.10

# CONFIG += staticlib
CONFIG += link_pkgconfig
PKGCONFIG += dbus-1
INCLUDEPATH += ../utility
DEPENDPATH += ../utility
include(../utility/utility.pri)
include(libsquid.pri)
LIBS += -ldc1394 \
    -lraw1394 \
    -lboost_filesystem \
    -lglog \
    -ltiff

QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
