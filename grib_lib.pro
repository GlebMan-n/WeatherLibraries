unix {
VERSION = 4.2.8
}

TEMPLATE = lib
CONFIG += qt shared warn_off
win32 {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = stdafx.h
}

QT += core sql network widgets gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
DEFINES += GRIB_LIB_LIB

DESTDIR = ../../../lib
LIBS += -L../../../lib

INCLUDEPATH +=  \
  ./ \
  ../ \
  ../../ \
  ../../../ \

QMAKE_CC += -std=c++0x

win32 {
  DEFINES -= UNICODE
  LIBS += -lodbc32 -ladvapi32
}

release {
  TARGET = grib_lib
  OBJECTS_DIR = ./release
  LIBS += -lgisbase -lagg -lQtImgDev -lMmfMatrix
  LIBS += -lGisSvd -lQtExtendUi -lPropertyBrowserQT
  LIBS += -lGdiGis -ljuce -lMmfMatrix -lGisGrib
}

debug {
  DEFINES +=_DEBUG
  TARGET = grib_lib_d
  OBJECTS_DIR = ./debug
  LIBS += -lgisbase_d -lagg_d -lQtImgDev_d -lMmfMatrix_d
  LIBS += -lGisSvd_d -lQtExtendUi_d -lPropertyBrowserQT_d
  LIBS += -lGdiGis_d -ljuce_d -lMmfMatrix_d -lGisGrib_d
}

MOC_DIR = $$OBJECTS_DIR
RCC_DIR = $$OBJECTS_DIR
UI_DIR = $$OBJECTS_DIR

unix {
  QMAKE_CXX += $$system('xml2-config --cflags')
  LIBS += $$system('xml2-config --libs')

  CONFIG += link_pkgconfig
  PKGCONFIG += log4cplus
}

include(./grib_lib.pri)

