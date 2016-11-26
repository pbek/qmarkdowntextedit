INCLUDEPATH += $$PWD/

QT       += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    $$PWD/markdownhighlighter.cpp \
    $$PWD/qmarkdowntextedit.cpp \
    $$PWD/qtexteditsearchwidget.cpp

RESOURCES += \
    $$PWD/media.qrc

FORMS += $$PWD/qtexteditsearchwidget.ui
