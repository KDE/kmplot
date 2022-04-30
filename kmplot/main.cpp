/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 1998, 1999, 2000, 2002 Klaus-Dieter Möller <kd.moeller@t-online.de>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

// local includes
#include "kmplot.h"
#include "kmplot_version.h"

// Qt
#include <QApplication>
#include <QCommandLineParser>

// KDE includes
#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>

int main(int argc, char **argv)
{
    /**
     * enable high dpi support
     */
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

    QApplication qapp(argc, argv);
    KCrash::initialize();

    KLocalizedString::setApplicationDomain("kmplot");

    KAboutData aboutData(QStringLiteral("kmplot"),
                         i18n("KmPlot"),
                         KMPLOT_VERSION_STRING,
                         i18n("Mathematical function plotter by KDE"),
                         KAboutLicense::GPL,
                         i18n("(c) 2000-2002, Klaus-Dieter Möller"),
                         QString(),
                         QStringLiteral("https://kde.org/applications/education/kmplot"));

    aboutData.addAuthor(i18n("Klaus-Dieter Möller"), i18n("Original Author"), QStringLiteral("kdmoeller@foni.net"));
    aboutData.addAuthor(i18n("Matthias Meßmer"), i18n("GUI"), QStringLiteral("bmlmessmer@web.de"));
    aboutData.addAuthor(i18n("Fredrik Edemar"), i18n("Various improvements"), QStringLiteral("f_edemar@linux.se"));
    aboutData.addAuthor(i18n("David Saxton"), i18n("Porting to Qt 4, UI improvements, features"), QStringLiteral("david@bluehaze.org"));

    aboutData.addCredit(i18n("David Vignoni"), i18n("svg icon"), QStringLiteral("david80v@tin.it"));
    aboutData.addCredit(i18n("Albert Astals Cid"), i18n("command line options, MIME type"), QStringLiteral("aacid@kde.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList{"function", "f"}, i18n("Initial functions to plot"), i18n("argument")));
    parser.addPositionalArgument(QStringLiteral("URL"), i18n("URLs to open"), QStringLiteral("[url...]"));
    aboutData.setupCommandLine(&parser);
    parser.process(qapp);
    aboutData.processCommandLine(&parser);
    KDBusService service;
    new KmPlot(parser);
    return qapp.exec();
}
