cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "08")
set (RELEASE_SERVICE_VERSION_MICRO "1")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

project(kmplot VERSION "1.3.${RELEASE_SERVICE_COMPACT_VERSION}")

set (QT_MIN_VERSION "5.6.0")
set (KF5_MIN_VERSION "5.69.0")

find_package (ECM 1.3.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(CheckIncludeFiles)
include(KDEInstallDirs)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Svg Widgets PrintSupport)

find_package (KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Crash
    GuiAddons
    I18n
    Parts
    WidgetsAddons
    DocTools
    DBusAddons
)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)

check_include_files(ieeefp.h HAVE_IEEEFP_H)
configure_file(config-kmplot.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kmplot/config-kmplot.h)
ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX KMPLOT VERSION_HEADER kmplot_version.h)

add_subdirectory( doc )
add_subdirectory( kmplot )
add_subdirectory( icons )

ki18n_install(po)
if (KF5DocTools_FOUND)
    kdoctools_install(po)
endif()
install(FILES org.kde.kmplot.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
