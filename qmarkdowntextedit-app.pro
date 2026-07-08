TARGET   = QMarkdownTextedit
TEMPLATE = app
QT += core gui widgets
CONFIG += c++11

macx {
    # Newer macOS SDKs no longer ship AGL.framework, but some qmake mkspecs
    # still add it through the default OpenGL linker flags. This demo app does
    # not use OpenGL directly, so avoid linking the obsolete framework.
    QMAKE_LIBS_OPENGL =
    QMAKE_LIBS_OPENGL_QT =
    QMAKE_LIBS_OPENGL_ES2 =
}

SOURCES = main.cpp mainwindow.cpp
HEADERS = mainwindow.h
FORMS   = mainwindow.ui

LIBS += -lQMarkdownTextedit -L$$OUT_PWD

win32: LIBS +=  -L$$OUT_PWD/release -L$$OUT_PWD/debug

target.path = $$[QT_INSTALL_BINS]

INSTALLS += target
