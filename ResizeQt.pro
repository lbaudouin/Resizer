#-------------------------------------------------
#
# Project created by QtCreator 2012-10-21T14:17:11
#
#-------------------------------------------------

QT       += core gui

TARGET = ResizeQt
TEMPLATE = app

LIBS += -lexiv2 `pkg-config opencv --libs --cflags`

SOURCES += main.cpp\
        resize.cpp

HEADERS  += resize.h

FORMS    += resize.ui

include(qtsingleapplication/qtsingleapplication.pri)
