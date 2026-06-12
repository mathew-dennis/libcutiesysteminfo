cmake_minimum_required(VERSION 3.16)
project(cutiesysteminfo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick)
qt_standard_project_setup()

set(lib_SOURCES
    src/cutiesysteminfo.h
    src/cutiesysteminfo.cpp)

set(plugin_SOURCES
    src/plugin.cpp src/plugin.h)

qt6_add_library(cutiesysteminfo
    SHARED ${lib_SOURCES}
)

qt6_add_qml_module(qmlcutiesysteminfoplugin
    URI Cutie.SystemInfo
    VERSION ${PROJECT_VERSION}
    PLUGIN_TARGET qmlcutiesysteminfoplugin
    NO_GENERATE_PLUGIN_SOURCE
    SOURCES
        ${plugin_SOURCES}
)

set_target_properties(cutiesysteminfo PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(cutiesysteminfo PROPERTIES SOVERSION 0)
set_target_properties(cutiesysteminfo PROPERTIES PUBLIC_HEADER src/cutiesysteminfo.h)

target_link_libraries(cutiesysteminfo PUBLIC
    Qt::Core
    Qt::Quick
)

target_link_libraries(qmlcutiesysteminfoplugin PUBLIC
    Qt::Core
    Qt::Quick
    cutiesysteminfo
)

configure_file(src/cutiesysteminfo.pc.in cutiesysteminfo.pc @ONLY)

install(TARGETS cutiesysteminfo
    RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    BUNDLE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(FILES ${CMAKE_BINARY_DIR}/cutiesysteminfo.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

install(TARGETS qmlcutiesysteminfoplugin
    RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt6/qml/Cutie/Systeminfo"
    BUNDLE DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt6/qml/Cutie/Systeminfo"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt6/qml/Cutie/Systeminfo"
)

install(DIRECTORY src/qml/Cutie/Systeminfo DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt6/qml/Cutie")
