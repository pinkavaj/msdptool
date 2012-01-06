#-------------------------------------------------
#
# Project created by QtCreator 2011-10-13T15:57:45
#
#-------------------------------------------------

QT       -= gui

TARGET = msdptool
TEMPLATE = lib
# comment/uncomment to build dinamic/static library
CONFIG += staticlib

INCLUDEPATH += ../src/include/

VERSION = 0.0

#DEFINES += MSDPTOOL_LIBRARY

SOURCES += \
    ../src/msdp2xxx_low.c \
    ../src/msdp2xxx.c

HEADERS += \
    ../src/include/msdp2xxx_low.h \
    ../src/include/msdp2xxx_base.h \
    ../src/include/msdp2xxx.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES +=
