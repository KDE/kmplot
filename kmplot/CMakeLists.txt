
add_definitions(-DTRANSLATION_DOMAIN=\"kmplot\")


########### next target ###############

set(kmplotpart_PART_SRCS 
   plotstylewidget.cpp
   initialconditionseditor.cpp
   functioneditor.cpp
   constants.cpp
   parser.cpp 
   xparser.cpp 
   equationedit.cpp
   equationeditwidget.cpp
   equationhighlighter.cpp
   equationeditorwidget.cpp
   equationeditor.cpp
   function.cpp
   view.cpp 
   maindlg.cpp 
   kprinterdlg.cpp 
   kconstanteditor.cpp
   kparametereditor.cpp
   functiontools.cpp 
   kmplotio.cpp 
   ksliderwindow.cpp 
   parameterswidget.cpp
   coordsconfigdialog.cpp
   parameteranimator.cpp
   vector.cpp
   kgradientdialog.cpp
   calculator.cpp
   )

qt5_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.MainDlg.xml maindlg.h MainDlg)
qt5_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.Parser.xml xparser.h XParser)
qt5_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.View.xml view.h View)

ki18n_wrap_ui(kmplotpart_PART_SRCS
	plotstylewidget.ui
	initialconditionswidget.ui
	functioneditorwidget.ui
	editcoords.ui
	functiontools.ui
	qparametereditor.ui
	constantseditor.ui
	settingspagecolor.ui
	settingspagefonts.ui
	settingspagegeneral.ui
	settingspagediagram.ui
	sliderwidget.ui
	parameterswidget.ui
	parameteranimator.ui
	equationeditorwidget.ui
	)

kconfig_add_kcfg_files(kmplotpart_PART_SRCS settings.kcfgc )

add_library(kmplotpart MODULE ${kmplotpart_PART_SRCS})



target_link_libraries(kmplotpart KF5::KDELibs4Support KF5::Parts Qt5::Svg)

install(TARGETS kmplotpart  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############   

set(kmplot_SRCS main.cpp kmplot.cpp kmplotprogress.cpp)
qt5_add_dbus_adaptor(kmplot_SRCS org.kde.kmplot.KmPlot.xml kmplot.h KmPlot)

# FIXME: it's wrong, but otherwise it does not compile
kconfig_add_kcfg_files(kmplot_SRCS settings.kcfgc )

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-kmplot.png")
ecm_add_app_icon(kmplot_SRCS ICONS ${ICONS_SRCS})

add_executable(kmplot ${kmplot_SRCS})

target_link_libraries(kmplot KF5::Crash KF5::KDELibs4Support KF5::Parts)

install(TARGETS kmplot  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS org.kde.kmplot.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmplot.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES kmplot_part.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kmplot_part.rc kmplot_part_readonly.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/kmplot )
install( FILES kmplot_shell.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/kmplot )
install( FILES org.kde.kmplot.KmPlot.xml org.kde.kmplot.MainDlg.xml org.kde.kmplot.Parser.xml org.kde.kmplot.View.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )


