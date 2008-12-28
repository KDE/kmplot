



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

qt4_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.MainDlg.xml maindlg.h MainDlg)
qt4_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.Parser.xml xparser.h XParser)
qt4_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.View.xml view.h View)

kde4_add_ui_files(kmplotpart_PART_SRCS
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
	equationeditor.ui
	)

kde4_add_kcfg_files(kmplotpart_PART_SRCS settings.kcfgc )

kde4_add_plugin(kmplotpart WITH_PREFIX ${kmplotpart_PART_SRCS})



target_link_libraries(kmplotpart  ${KDE4_KPARTS_LIBS} ${QT_QTXML_LIBRARY} )

install(TARGETS kmplotpart  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############   

set(kmplot_SRCS main.cpp kmplot.cpp kmplotprogress.cpp)
qt4_add_dbus_adaptor(kmplot_SRCS org.kde.kmplot.KmPlot.xml kmplot.h KmPlot)

# FIXME: it's wrong, but otherwise it does not compile
kde4_add_kcfg_files(kmplot_SRCS settings.kcfgc )

#kde4_add_app_icon(kmplot_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/kmplot.png")
kde4_add_app_icon(kmplot_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-apps-kmplot.png")

kde4_add_executable(kmplot ${kmplot_SRCS})

target_link_libraries(kmplot  ${KDE4_KPARTS_LIBS})

install(TARGETS kmplot  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES kmplot.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmplot.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES kmplot_part.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kmplot_part.rc kmplot_part_readonly.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmplot )
install( FILES kmplot_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmplot )
install( FILES org.kde.kmplot.KmPlot.xml org.kde.kmplot.MainDlg.xml org.kde.kmplot.Parser.xml org.kde.kmplot.View.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )


