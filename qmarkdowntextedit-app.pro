TARGET   = QMarkdownTextedit
TEMPLATE = app
QT += core gui widgets
CONFIG += c++11

SOURCES = main.cpp mainwindow.cpp
HEADERS = mainwindow.h
FORMS   = mainwindow.ui

LIBS += -lQMarkdownTextedit -L$$OUT_PWD

target.path = $$[QT_INSTALL_BINS]

INSTALLS += target

