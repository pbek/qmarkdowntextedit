#-------------------------------------------------
#
# Project created by QtCreator 2016-01-11T16:56:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QMarkdownTextedit
TEMPLATE = app
TRANSLATIONS = trans/qmarkdowntextedit_de.ts
CONFIG += c++11

SOURCES += main.cpp \
    mainwindow.cpp \

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

include(qmarkdowntextedit.pri)
