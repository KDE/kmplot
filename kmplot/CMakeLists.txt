
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

qt_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.MainDlg.xml maindlg.h MainDlg)
qt_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.Parser.xml xparser.h XParser)
qt_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.View.xml view.h View)

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
kcoreaddons_desktop_to_json(kmplotpart kmplot_part.desktop)



target_link_libraries(kmplotpart Qt::PrintSupport Qt::Widgets KF${QT_MAJOR_VERSION}::Parts Qt::Svg)

install(TARGETS kmplotpart DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/parts)

########### next target ###############   

set(kmplot_SRCS main.cpp kmplot.cpp kmplotprogress.cpp)
qt_add_dbus_adaptor(kmplot_SRCS org.kde.kmplot.KmPlot.xml kmplot.h KmPlot)

# FIXME: it's wrong, but otherwise it does not compile
kconfig_add_kcfg_files(kmplot_SRCS settings.kcfgc )

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-kmplot.png")
ecm_add_app_icon(kmplot_SRCS ICONS ${ICONS_SRCS})

add_executable(kmplot ${kmplot_SRCS})

target_link_libraries(kmplot Qt::PrintSupport Qt::Widgets KF${QT_MAJOR_VERSION}::Crash KF${QT_MAJOR_VERSION}::Parts KF${QT_MAJOR_VERSION}::DBusAddons)

install(TARGETS kmplot  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS org.kde.kmplot.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES kmplot.kcfg  DESTINATION  ${KDE_INSTALL_KCFGDIR} )
install( FILES kmplot_part.desktop  DESTINATION  ${KDE_INSTALL_KSERVICESDIR} )
install( FILES kmplot_part.rc kmplot_part_readonly.rc  DESTINATION  ${KDE_INSTALL_KXMLGUIDIR}/kmplot )
install( FILES kmplot_shell.rc  DESTINATION  ${KDE_INSTALL_KXMLGUIDIR}/kmplot )
install( FILES org.kde.kmplot.KmPlot.xml org.kde.kmplot.MainDlg.xml org.kde.kmplot.Parser.xml org.kde.kmplot.View.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} )


