TARGET   = QMarkdownTextedit
TEMPLATE = app
QT += core gui widgets
CONFIG += c++11

macx {
    # Newer macOS SDKs no longer ship AGL.framework, but some qmake mkspecs
    # still add it through the default OpenGL linker flags. Keep this non-empty
    # for older Qt 6 qmake mkspecs that restore the default when it is empty.
    QMAKE_LIBS_OPENGL = -framework OpenGL
    QMAKE_LIBS_OPENGL_QT = -framework OpenGL
    QMAKE_LIBS_OPENGL_ES2 =

    greaterThan(QT_MAJOR_VERSION, 5) {
        # Older Qt 6 macOS .prl files still pull in the removed AGL framework.
        CONFIG -= link_prl

        # Qt 6 headers call the ARM yield intrinsic without including arm_acle.h
        # on macOS 26 ARM runners.
        contains(QMAKE_HOST.arch, arm64) {
            QMAKE_CXXFLAGS += -include arm_acle.h
        }
    }
}

SOURCES = main.cpp mainwindow.cpp
HEADERS = mainwindow.h
FORMS   = mainwindow.ui

LIBS += -lQMarkdownTextedit -L$$OUT_PWD

win32: LIBS +=  -L$$OUT_PWD/release -L$$OUT_PWD/debug

target.path = $$[QT_INSTALL_BINS]

INSTALLS += target
