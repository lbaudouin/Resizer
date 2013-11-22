SOURCES += resizer_unity.cpp

HEADERS += plugininterface.h \
           resizer_unity.h

CONFIG += link_pkgconfig
PKGCONFIG += unity

TEMPLATE = lib
CONFIG += plugin
TARGET = unity-plugin
