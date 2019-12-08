TARGET   = QMarkdownTextedit
TEMPLATE = app
QT += core gui widgets

SOURCES = main.cpp mainwindow.cpp
HEADERS = mainwindow.h
FORMS   = mainwindow.ui

LIBS += -lQMarkdownTextedit -L$$OUT_PWD

target.path = $$[QT_INSTALL_BINS]

INSTALLS += target

