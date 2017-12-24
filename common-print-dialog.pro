QT       += core gui widgets printsupport

TARGET = common-print-dialog
TEMPLATE = lib

DEFINES += COMMONPRINTDIALOG_LIBRARY

SOURCES += \
        src/qcommonprintdialog.cpp

HEADERS += \
        src/qcommonprintdialog.h \
        src/common-print-dialog_global.h

unix {
    target.path = /usr/local
    headerfiles.path = /usr/local/include/common-print-dialog
    headerfiles.files = $$PWD/src/*.h
    INSTALLS += target headerfiles
}
