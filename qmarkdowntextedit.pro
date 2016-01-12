#-------------------------------------------------
#
# Project created by QtCreator 2016-01-11T16:56:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QMarkdownTextedit
TEMPLATE = app


SOURCES += main.cpp \
    mainwindow.cpp \
    lib/peg-markdown-highlight/pmh_parser.c \
    highlighter.cpp \
    qmarkdowntextedit.cpp \
    qtexteditsearchwidget.cpp

HEADERS  += mainwindow.h \
    lib/peg-markdown-highlight/pmh_definitions.h \
    lib/peg-markdown-highlight/pmh_parser.h \
    highlighter.h \
    qmarkdowntextedit.h \
    qtexteditsearchwidget.h

FORMS    += mainwindow.ui
