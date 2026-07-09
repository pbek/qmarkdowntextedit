TARGET = QMarkdownTextedit
TEMPLATE = lib
QT += core gui widgets
CONFIG += c++11 create_prl no_install_prl create_pc

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

include(qmarkdowntextedit.pri)

TRANSLATIONS += trans/qmarkdowntextedit_de.ts \
                trans/qmarkdowntextedit_zh_CN.ts \
                trans/qmarkdowntextedit_es.ts

isEmpty(PREFIX):PREFIX=$$[QT_INSTALL_PREFIX]
isEmpty(LIBDIR):LIBDIR=$$[QT_INSTALL_LIBS]
isEmpty(HEADERDIR):HEADERDIR=$${PREFIX}/include/$$TARGET/
isEmpty(DSRDIR):DSRDIR=$${PREFIX}/share/$$TARGET

target.path = $${LIBDIR}

headers.files = $$HEADERS
headers.path = $${HEADERDIR}

license.files = LICENSE
license.path = $${DSRDIR}/licenses/

trans.files = trans/*.qm
trans.path = $${DSRDIR}/translations/

QMAKE_PKGCONFIG_NAME = QMarkdownTextedit
QMAKE_PKGCONFIG_DESCRIPTION = C++ Qt QPlainTextEdit widget with markdown highlighting and some other goodies
QMAKE_PKGCONFIG_INCDIR = $${headers.path}
QMAKE_PKGCONFIG_LIBDIR = $${LIBDIR}
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

INSTALLS += target license headers trans
