cmake_minimum_required(VERSION 3.16)

project(Bina VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Sql)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appBina
        src/cpp/main.cpp
)

qt_add_qml_module(appBina
    URI Bina
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
        src/qml/Login.qml
    RESOURCES
        resource/resources.qrc
        SOURCES src/cpp/databasemanager.h src/cpp/databasemanager.cpp
)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appBina PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appBina
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appBina
    PRIVATE Qt6::Quick
            Qt6::Sql
)

include(GNUInstallDirs)
install(TARGETS appBina
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
