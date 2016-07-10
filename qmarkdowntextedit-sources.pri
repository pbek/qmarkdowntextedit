INCLUDEPATH += $$PWD/

QT       += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    $$PWD/lib/peg-markdown-highlight/pmh_parser.c \
    $$PWD/markdownhighlighter.cpp \
    $$PWD/qmarkdowntextedit.cpp \
    $$PWD/qtexteditsearchwidget.cpp

RESOURCES += \
    $$PWD/media.qrc

FORMS += $$PWD/qtexteditsearchwidget.ui
