QT       += widgets

TARGET = common-print-dialog
TEMPLATE = lib

DEFINES += COMMONPRINTDIALOG_LIBRARY

SOURCES += \
        src/qcommonprintdialog.cpp

HEADERS += \
        src/qcommonprintdialog.h \
        src/common-print-dialog_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
