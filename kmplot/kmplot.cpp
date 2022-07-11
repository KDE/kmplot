/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "kmplot.h"

#include <QFileDialog>
#include <QLabel>
#include <QStatusBar>
#include <QUrl>

#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KDialogJobUiDelegate>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KToggleFullScreenAction>

#include "kmplotadaptor.h"
#include "maindlg.h"
#include "view.h"
#include <kmplotprogress.h>

static QUrl urlFromArg(const QString &arg)
{
    return QUrl::fromUserInput(arg, QDir::currentPath(), QUrl::AssumeLocalFile);
}

KmPlot::KmPlot(const QCommandLineParser &parser)
    : KParts::MainWindow()
{
    setObjectName(QStringLiteral("KmPlot"));

    // set the shell's ui resource file
    setXMLFile(QStringLiteral("kmplot_shell.rc"));
    // then, setup our actions
    setupActions();

    // setup the status bar
    setupStatusBar();

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    const auto result = KPluginFactory::instantiatePlugin<KParts::ReadWritePart>(KPluginMetaData(QStringLiteral("kf5/parts/kmplotpart")), this);
    if (result) {
        m_part = result.plugin;
        // tell the KParts::MainWindow that this is indeed the main widget
        setCentralWidget(m_part->widget());
        // m_part->widget()->setFocus();
        //  and integrate the part's GUI with the shell's
        setupGUI(Keys | ToolBar | Save);
        createGUI(m_part);
    } else {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Could not find KmPlot's part."));
        qApp->quit();
        // we return here, cause qApp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();
    {
        bool exit = false;
        bool first = true;
        const auto arguments = parser.positionalArguments();
        for (const QString &arg : arguments) {
            QUrl url = urlFromArg(arg);
            if (first) {
                exit = !load(url);
            } else
                openFileInNewWindow(url);
        }
        if (exit)
            deleteLater(); // couldn't open the file, and therefore exit
        first = false;
    }

    show();

    new KmPlotAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/kmplot"), this);

    if (parser.isSet(QStringLiteral("function"))) {
        QString f = parser.value(QStringLiteral("function"));
        QDBusReply<bool> reply = QDBusInterface(QDBusConnection::sessionBus().baseService(), QStringLiteral("/parser"), QStringLiteral("org.kde.kmplot.Parser"))
                                     .call(QDBus::BlockWithGui, QStringLiteral("addFunction"), f, "");
    }
}

KmPlot::~KmPlot()
{
}

void KmPlot::slotUpdateFullScreen(bool checked)
{
    if (checked) {
        KToggleFullScreenAction::setFullScreen(this, true);
        // m_fullScreen->plug( toolBar( "mainToolBar" ) ); deprecated annma 2006-03-01
    } else {
        KToggleFullScreenAction::setFullScreen(this, false);
        // m_fullScreen->unplug( toolBar( "mainToolBar" ) ); deprecated annma 2006-03-01
    }
}

bool KmPlot::load(const QUrl &url)
{
    m_part->openUrl(url);
    if (m_part->url().isEmpty())
        return false;
    setWindowTitle(url.toDisplayString());
    return true;
}

void KmPlot::setupActions()
{
    KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
    KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
    KStandardAction::quit(this, SLOT(close()), actionCollection());

    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);

    m_fullScreen = KStandardAction::fullScreen(NULL, NULL, this, actionCollection());
    actionCollection()->addAction(QStringLiteral("fullscreen"), m_fullScreen);
    connect(m_fullScreen, &KToggleFullScreenAction::toggled, this, &KmPlot::slotUpdateFullScreen);
}

void KmPlot::fileNew()
{
    // About this function, the style guide
    // says that it should open a new window if the document is _not_
    // in its initial state.  This is what we do here...
    if (!m_part->url().isEmpty() || isModified()) {
        KIO::CommandLauncherJob *job = new KIO::CommandLauncherJob(QStringLiteral("kmplot"), this);
        job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
        job->start();
    }
}

void KmPlot::applyNewToolbarConfig()
{
    applyMainWindowSettings(KSharedConfig::openConfig()->group(QString()));
}

void KmPlot::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QUrl const url = QFileDialog::getOpenFileUrl(this, i18n("Open"), QUrl::fromLocalFile(QDir::currentPath()), i18n("KmPlot Files (*.fkt);;All Files (*)"));

    if (!url.isEmpty()) {
        // About this function, the style guide
        // says that it should open a new window if the document is _not_
        // in its initial state.  This is what we do here..
        if (m_part->url().isEmpty() && !isModified())
            load(url); // we open the file in this window...
        else
            openFileInNewWindow(url); // we open the file in a new window...
    }
}

void KmPlot::fileOpen(const QUrl &url)
{
    if (!url.isEmpty()) {
        // About this function, the style guide
        // says that it should open a new window if the document is _not_
        // in its initial state.  This is what we do here..
        if (m_part->url().isEmpty() && !isModified())
            load(url); // we open the file in this window...
        else
            openFileInNewWindow(url); // we open the file in a new window...
    }
}

void KmPlot::openFileInNewWindow(const QUrl &url)
{
    KIO::CommandLauncherJob *job = new KIO::CommandLauncherJob(QStringLiteral("kmplot"), {url.url()}, this);
    job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
    job->start();
}

bool KmPlot::isModified()
{
    QDBusReply<bool> reply = QDBusInterface(QDBusConnection::sessionBus().baseService(), QStringLiteral("/maindlg"), QStringLiteral("org.kde.kmplot.MainDlg"))
                                 .call(QDBus::BlockWithGui, QStringLiteral("isModified"));
    return reply.value();
}

bool KmPlot::queryClose()
{
    return m_part->queryClose();
}

void KmPlot::setStatusBarText(const QString &text, int id)
{
    static_cast<QLabel *>(statusBarLabels.at(id))->setText(text);
}

void KmPlot::setupStatusBar()
{
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    for (int i = 0; i < View::SectionCount; ++i) {
        QLabel *label = new QLabel(statusBar);
        label->setFixedHeight(label->fontMetrics().height() + 2);
        /// Labels for coordinates should be of fixed width 16 chars to be the same as for good old KmPlot
        if (i < 2) {
            label->setFixedWidth(label->fontMetrics().boundingRect(QLatin1Char('8')).width() * 16);
            label->setAlignment(Qt::AlignCenter);
        } else {
            label->setAlignment(Qt::AlignLeft);
        }

        statusBar->addWidget(label);
        statusBarLabels.append(label);
    }

    m_progressBar = new KmPlotProgress(statusBar);
    m_progressBar->setMaximumHeight(statusBar->height() - 10);
    connect(m_progressBar, &KmPlotProgress::cancelDraw, this, &KmPlot::cancelDraw);
    statusBar->addWidget(m_progressBar);
}

void KmPlot::setDrawProgress(double progress)
{
    m_progressBar->setProgress(progress);
}

void KmPlot::cancelDraw()
{
    QDBusInterface(QDBusConnection::sessionBus().baseService(), QStringLiteral("/kmplot"), QStringLiteral("org.kde.kmplot.KmPlot"))
        .call(QDBus::NoBlock, QStringLiteral("stopDrawing"));
}
