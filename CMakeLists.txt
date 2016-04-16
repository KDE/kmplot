project(kmplot)
cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.3.0")

find_package (ECM 1.3.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(CheckIncludeFiles)
include(KDEInstallDirs)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Svg Widgets PrintSupport)

find_package (KF5 REQUIRED
    Crash
    GuiAddons
    I18n
    Parts
    WidgetsAddons
    DocTools
    KDELibs4Support
)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

check_include_files(ieeefp.h HAVE_IEEEFP_H)
configure_file(config-kmplot.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kmplot/config-kmplot.h)

add_subdirectory( doc )
add_subdirectory( kmplot )
add_subdirectory( icons )

install(FILES org.kde.kmplot.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/)
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
