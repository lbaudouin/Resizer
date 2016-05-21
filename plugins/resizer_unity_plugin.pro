SOURCES += resizer_unity.cpp

HEADERS += plugininterface.h\
           resizer_unity.h

CONFIG += link_pkgconfig
PKGCONFIG += unity

CONFIG += c++11

TEMPLATE = lib
CONFIG += plugin
TARGET = resizer-plugin-unity
#TARGET = $$qtLibraryTarget(resizer-plugin-unity)
