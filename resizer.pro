#-------------------------------------------------
#
# Project created by QtCreator 2012-10-21T14:17:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent

TARGET = resizer
TEMPLATE = app

CONFIG += c++11

TRANSLATIONS = resizer_fr.ts

SOURCES += main.cpp\
    qexifimageheader/qexifimageheader.cpp \
    resizer.cpp \
    positionselector.cpp \
    imagelabel.cpp \
    zip/zip.cpp

HEADERS  += \
    qexifimageheader/qexifimageheader.h \
    resizer.h \
    positionselector.h \
    plugininterface.h \
    rotationstate.h \
    imagelabel.h \
    zip/zipreader.h \
    zip/zipwriter.h

FORMS    += \
    resizer.ui

include(singleapplication.pri)

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

contains( QT_CONFIG, system-zlib ) {
  if( unix|win32-g++* ): LIBS += -lz
  else: LIBS += zdll.lib
} else {
  INCLUDEPATH += $$PWD
}

DISTFILES += \
    debian/control \
    debian/changelog \
    debian/rules \
    debian/copyright \
    data/usr/share/applications/resizer.desktop \
    data/usr/share/icons/hicolor/16x16/apps/resizer.png \
    data/usr/share/icons/hicolor/32x32/apps/resizer.png \
    data/usr/share/icons/hicolor/64x64/apps/resizer.png \
    data/usr/share/menu/resizer
