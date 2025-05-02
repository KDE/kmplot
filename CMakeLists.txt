cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "04")
set (RELEASE_SERVICE_VERSION_MICRO "1")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

project(kmplot VERSION "1.3.${RELEASE_SERVICE_COMPACT_VERSION}")

set (QT_MIN_VERSION "6.6")
set (KF6_MIN_VERSION "6.1.0")
set (KDE_COMPILERSETTINGS_LEVEL "5.82.0")

find_package (ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(CheckIncludeFiles)
include(KDEInstallDirs)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)

option(BUILD_DOC "Whether to build the documentation" ON)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Svg Widgets PrintSupport)

find_package (KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Completion
    Config
    Crash
    DBusAddons
    GuiAddons
    I18n
    JobWidgets
    KIO
    Parts
    WidgetsAddons
    TextWidgets
)

if (BUILD_DOC)
    find_package(KF6DocTools ${KF6_MIN_VERSION})
    set_package_properties(KF6DocTools PROPERTIES
        DESCRIPTION "Tools to generate documentation"
        TYPE REQUIRED
    )
endif()

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)

check_include_files(ieeefp.h HAVE_IEEEFP_H)
configure_file(config-kmplot.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kmplot/config-kmplot.h)
ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX KMPLOT VERSION_HEADER kmplot_version.h)

add_subdirectory( kmplot )
add_subdirectory( icons )

ki18n_install(po)
if (BUILD_DOC)
    add_subdirectory(doc)
    kdoctools_install(po)
endif()
install(FILES org.kde.kmplot.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
