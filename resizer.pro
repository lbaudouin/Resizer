#-------------------------------------------------
#
# Project created by QtCreator 2012-10-21T14:17:11
#
#-------------------------------------------------

QT       += core gui

TARGET = resizer
TEMPLATE = app

TRANSLATIONS = resizer_fr.ts

SOURCES += main.cpp\
    qexifimageheader/qexifimageheader.cpp \
    resizer.cpp \
    positionselector.cpp \
    imagelabel.cpp

HEADERS  += \
    qexifimageheader/qexifimageheader.h \
    resizer.h \
    positionselector.h \
    plugininterface.h \
    rotationstate.h \
    imagelabel.h

FORMS    += \
    resizer.ui

include(qtsingleapplication/qtsingleapplication.pri)

RESOURCES += \
    ressources.qrc

win32 {
    HEADERS += updatemanager/updatemanager.h \
               updatemanager/qprogressbardialog.h
    SOURCES += updatemanager/updatemanager.cpp
    TARGET = Resizer
    RC_FILE = resizer.rc
}

OTHER_FILES += \
    resizer.rc \
    TODO \
    win32/Resizer.exe.manifest
