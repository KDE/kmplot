/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "kmplotprogress.h"

#include <QCoreApplication>
#include <QDebug>
#include <QIcon>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>

KmPlotProgress::KmPlotProgress(QWidget *parent)
    : QWidget(parent)
{
    m_button = new QPushButton(this);
    m_button->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
    m_button->setGeometry(QRect(0, 0, 30, 23));
    m_button->setMaximumHeight(height() - 10);
    connect(m_button, &QPushButton::clicked, this, &KmPlotProgress::cancelDraw);

    m_progress = new QProgressBar(this);
    m_progress->setGeometry(QRect(30, 0, 124, 23));
    m_progress->setMaximumHeight(height() - 10);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, this, &KmPlotProgress::showProgressBar);

    hide();
    setMinimumWidth(154);
}

KmPlotProgress::~KmPlotProgress()
{
}

void KmPlotProgress::showProgressBar()
{
    show();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void KmPlotProgress::setProgress(double progress)
{
    qDebug() << "progress=" << progress;

    Q_ASSERT(progress >= 0);

    if (progress >= 1) {
        hide();
        m_showTimer->stop();
    } else {
        if (!isVisible() && !m_showTimer->isActive())
            m_showTimer->start(500);

        m_progress->setValue(int(progress * 100));
    }

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}
