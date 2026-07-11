#-------------------------------------------------
#
# Project created by QtCreator 2026-07-07T16:23:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Algography
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    getstarteddlg.cpp \
    newdialog.cpp \
    algohighlighter.cpp \
    completereventfilter.cpp \
    settingsdialog.cpp

HEADERS  += mainwindow.h \
    getstarteddlg.h \
    newdialog.h \
    algohighlighter.h \
    completereventfilter.h \
    settingsdialog.h

FORMS    += mainwindow.ui \
    getstarteddlg.ui \
    newdialog.ui \
    settingsdialog.ui

RESOURCES += \
    res.qrc

win32:RC_ICONS = algography.ico
