TARGET = QMarkdownTextedit
TEMPLATE = lib
QT += core gui widgets
CONFIG += c++11

include(qmarkdowntextedit.pri)

TRANSLATIONS += trans/qmarkdowntextedit_de.ts \
                trans/qmarkdowntextedit_zh_CN.ts

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

INSTALLS += target license headers trans
