QT       += widgets printsupport

TARGET = printTest
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++14

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L/usr/local/release/ -lcommon-print-dialog
else:win32:CONFIG(debug, debug|release): LIBS += -L/usr/local/debug/ -lcommon-print-dialog
else:unix: LIBS += -L/usr/local/ -lcommon-print-dialog

INCLUDEPATH += /usr/local/include/common-print-dialog
DEPENDPATH += /usr/local/include/common-print-dialog
