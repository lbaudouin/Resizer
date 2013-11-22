SOURCES += resizer_unity.cpp \
           resizer_unity.h

HEADERS += plugininterface.h\
           resizer_unity.h

CONFIG += link_pkgconfig
PKGCONFIG += unity

TEMPLATE = lib
CONFIG += plugin
TARGET = unity-plugin
#TARGET = $$qtLibraryTarget(unity-plugin)
