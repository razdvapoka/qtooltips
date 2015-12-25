QT += widgets

TARGET      = tooltipmanager
TEMPLATE    = lib

DESTDIR     =  build
MOC_DIR     = build/.moc
OBJECTS_DIR = build/.obj

DEFINES     += TOOLTIPMANAGER_LIBRARY

SOURCES     += tooltipmanager.cpp

HEADERS     += tooltipmanager.h
