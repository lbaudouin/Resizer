#-------------------------------------------------
#
# Project created by QtCreator 2012-10-21T14:17:11
#
#-------------------------------------------------

QT       += core gui #declarative

TARGET = ResizeQt
TEMPLATE = app

TRANSLATIONS = ResizeQt_fr.ts

#LIBS += -lexiv2 `pkg-config opencv --libs --cflags`

SOURCES += main.cpp\
        resize.cpp \
    qexifimageheader/qexifimageheader.cpp

HEADERS  += resize.h \
    qexifimageheader/qexifimageheader.h

FORMS    += resize.ui

include(qtsingleapplication/qtsingleapplication.pri)

RESOURCES += \
    ressources.qrc
