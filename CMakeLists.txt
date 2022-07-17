cmake_minimum_required(VERSION 3.3)
project(qmarkdowntextedit LANGUAGES CXX)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_CXX_STANDARD 11) Use this instead
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# add option to disable test executable
option(QMARKDOWNTEXTEDIT_EXE "Build test executable" ON)

# find qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg)

# needed for windows
if(WIN32)
    set(INTL_LDFLAGS -lintl)
endif(WIN32)

# QMarkdownTextEdit library
set(RC_FILES
    media.qrc
)

# Translations arent loaded so don't include them
set(TS_FILES
    trans/qmarkdowntextedit_de.ts
    trans/qmarkdowntextedit_ur.ts
    trans/qmarkdowntextedit_zh_CN.ts
)

set(QMARKDOWNTEXTEDIT_FILES
    markdownhighlighter.h
    markdownhighlighter.cpp
    linenumberarea.h
    qmarkdowntextedit.h
    qmarkdowntextedit.cpp
    qownlanguagedata.h
    qownlanguagedata.cpp
    qplaintexteditsearchwidget.h
    qplaintexteditsearchwidget.ui
    qplaintexteditsearchwidget.cpp
    media.qrc
)

add_library(qmarkdowntextedit ${QMARKDOWNTEXTEDIT_FILES})

target_link_libraries(qmarkdowntextedit Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg ${INTL_LDFLAGS})

# QMarkdownTextEdit executable
if(QMARKDOWNTEXTEDIT_EXE)
    set(SOURCE_FILES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
    )

    add_executable(qmarkdowntextedit-exe ${SOURCE_FILES})
    target_link_libraries(qmarkdowntextedit-exe Qt${QT_VERSION_MAJOR}::Widgets ${INTL_LDFLAGS} qmarkdowntextedit)
endif()
