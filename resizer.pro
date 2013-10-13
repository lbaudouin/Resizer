#-------------------------------------------------
#
# Project created by QtCreator 2012-10-21T14:17:11
#
#-------------------------------------------------

QT       += core gui #declarative

TARGET = resizer
TEMPLATE = app

TRANSLATIONS = resizer_fr.ts

#LIBS += -lexiv2 `pkg-config opencv --libs --cflags`

SOURCES += main.cpp\
    qexifimageheader/qexifimageheader.cpp \
    resizer.cpp

HEADERS  += \
    qexifimageheader/qexifimageheader.h \
    resizer.h

FORMS    += \
    resizer.ui

include(qtsingleapplication/qtsingleapplication.pri)

RESOURCES += \
    ressources.qrc
