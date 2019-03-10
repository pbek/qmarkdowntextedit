cmake_minimum_required(VERSION 3.3)
project(qmarkdowntextedit)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package( Qt5Core REQUIRED )
find_package( Qt5Widgets REQUIRED )
find_package( Qt5Gui REQUIRED )

qt5_wrap_ui(ui_qplaintexteditsearchwidget.h qplaintexteditsearchwidget.ui)

set(RESOURCE_FILES
        media.qrc
        )

qt5_add_resources(RESOURCE_ADDED ${RESOURCE_FILES})

set(SOURCE_FILES
    markdownhighlighter.cpp
    markdownhighlighter.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    qmarkdowntextedit.cpp
    qmarkdowntextedit.h
    qplaintexteditsearchwidget.ui
    qplaintexteditsearchwidget.cpp
    qplaintexteditsearchwidget.h
    )

add_executable(qmarkdowntextedit ${SOURCE_FILES} ${RESOURCE_ADDED})

include_directories(${Qt5Widgets_INCLUDES})

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# The Qt5Widgets_LIBRARIES variable also includes QtGui and QtCore
target_link_libraries(qmarkdowntextedit Qt5::Widgets)
