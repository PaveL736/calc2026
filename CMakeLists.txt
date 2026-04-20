cmake_minimum_required(VERSION 3.16)
project(calc2026 VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)
qt_standard_project_setup()

# ---------- ресурсы ----------
qt6_add_resources(APP_RESOURCES resources.qrc)

# ---------- исполняемый файл ----------
qt_add_executable(appcalc2026
    main.cpp
    ${APP_RESOURCES}
)

# ---------- QML-модуль ----------
qt_add_qml_module(appcalc2026
    URI  calc2026
    VERSION 1.0
    QML_FILES Main.qml
    QML_FILES
    SOURCES calculator.cpp
    SOURCES calculator.h
)

# ---------- остальные свойства ----------
set_target_properties(appcalc2026 PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
target_link_libraries(appcalc2026 PRIVATE Qt6::Quick)

include(GNUInstallDirs)
install(TARGETS appcalc2026
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
