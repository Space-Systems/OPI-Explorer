cmake_minimum_required(VERSION 3.19)
project(OPIExplorer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui OpenGL Charts)
set(CMAKE_AUTORCC ON)

# Set default OPI library paths.
set (OPI_INCLUDE_DIR "/usr/include" CACHE PATH "Path to OPI include directory")
set (OPI_LIBRARY "/usr/lib/libOPI.so" CACHE FILEPATH "Path to OPI library")

qt_standard_project_setup()

qt_add_executable(OPIExplorer
#    WIN32 MACOSX_BUNDLE
    main.cpp
    opiexplorermain.cpp
    dataparser.cpp
    auxfunctions.cpp
    opiexplorermain.ui
    dataparser.ui
    oeresource.qrc
    glcanvas.cpp
)

target_include_directories(OPIExplorer
    PRIVATE 
        ${OPI_INCLUDE_DIR}
)
target_link_libraries(OPIExplorer
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::OpenGLWidgets
        Qt::Charts
        ${OPI_LIBRARY}
)

include(GNUInstallDirs)

install(TARGETS OPIExplorer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET OPIExplorer
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})