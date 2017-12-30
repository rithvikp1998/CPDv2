QT       += core gui widgets printsupport

TARGET = common-print-dialog
TEMPLATE = lib

DEFINES += COMMONPRINTDIALOG_LIBRARY

CONFIG += c++11 no_keywords

SOURCES += \
        src/qcommonprintdialog.cpp

HEADERS += \
        src/qcommonprintdialog.h \
        src/common-print-dialog_global.h \
    src/singleton.h

unix {
    target.path = /usr/lib
    headerfiles.path = /usr/include/common-print-dialog
    headerfiles.files = $$PWD/src/*.h
    INSTALLS += target headerfiles
    CONFIG += link_pkgconfig
    PKGCONFIG += gio-unix-2.0 glib-2.0 gobject-2.0 cpdb-libs-frontend
}
