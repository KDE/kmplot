project(kmplot)
cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.3.0")

find_package (ECM 1.0.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(CheckIncludeFiles)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package (Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Svg Widgets)

find_package (KF5 REQUIRED
    GuiAddons
    I18n
    KDELibs4Support
    Parts
    WidgetsAddons
)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

check_include_files(ieeefp.h HAVE_IEEEFP_H)
configure_file(config-kmplot.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kmplot/config-kmplot.h)

#add_subdirectory( doc )
add_subdirectory( kmplot )
#add_subdirectory( icons )

