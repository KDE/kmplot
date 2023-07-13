/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "parameteranimator.h"
#include "ui_parameteranimator.h"
#include "view.h"

#include <KConfigGroup>

#include <QDialogButtonBox>
#include <QIcon>
#include <QTimer>

#include <assert.h>
#include <cmath>

using namespace std;

#ifndef KDEWIN_MATH_H
double inline log(int n)
{
    return log(double(n));
}
#endif

class ParameterAnimatorWidget : public QWidget, public Ui::ParameterAnimator
{
public:
    ParameterAnimatorWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

// BEGIN class ParameterAnimator
ParameterAnimator::ParameterAnimator(QWidget *parent, Function *function)
    : QDialog(parent)
    , m_function(function)
{
    m_widget = new ParameterAnimatorWidget(this);

    setWindowTitle(i18nc("@title:window", "Parameter Animator"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ParameterAnimator::reject);

    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(m_widget);
    dialogLayout->addWidget(buttonBox);

    m_mode = Paused;
    m_currentValue = 0;
    m_function->m_parameters.animating = true;
    m_function->k = m_currentValue;

    if (function->eq[0]->usesParameter())
        m_widget->warningLabel->hide();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ParameterAnimator::step);

    m_widget->gotoInitial->setIcon(QIcon::fromTheme("go-first"));
    m_widget->gotoFinal->setIcon(QIcon::fromTheme("go-last"));
    m_widget->stepBackwards->setIcon(QIcon::fromTheme("go-previous"));
    m_widget->stepForwards->setIcon(QIcon::fromTheme("go-next"));
    m_widget->pause->setIcon(QIcon::fromTheme("media-playback-pause"));

    connect(m_widget->gotoInitial, &QToolButton::clicked, this, &ParameterAnimator::gotoInitial);
    connect(m_widget->gotoFinal, &QToolButton::clicked, this, &ParameterAnimator::gotoFinal);
    connect(m_widget->stepBackwards, &QToolButton::toggled, this, &ParameterAnimator::stepBackwards);
    connect(m_widget->stepForwards, &QToolButton::toggled, this, &ParameterAnimator::stepForwards);
    connect(m_widget->pause, &QToolButton::clicked, this, &ParameterAnimator::pause);
    connect(m_widget->speed, &QSlider::valueChanged, this, &ParameterAnimator::updateSpeed);

    updateUI();
    updateFunctionParameter();

    connect(this, &ParameterAnimator::finished, this, &ParameterAnimator::deleteLater);
}

ParameterAnimator::~ParameterAnimator()
{
    qDebug();
    m_function->m_parameters.animating = false;
    View::self()->drawPlot();
}

void ParameterAnimator::step()
{
    // This function shouldn't get called when we aren't actually stepping
    assert(m_mode != Paused);

    double dx = m_widget->step->value();

    bool increasing = ((m_mode == StepBackwards && (dx < 0)) || (m_mode == StepForwards && (dx > 0)));
    bool decreasing = ((m_mode == StepBackwards && (dx > 0)) || (m_mode == StepForwards && (dx < 0)));

    double upper = m_widget->final->value();
    double lower = m_widget->initial->value();

    if (lower > upper)
        qSwap(lower, upper);

    if ((increasing && (m_currentValue >= upper)) || (decreasing && (m_currentValue <= lower))) {
        stopStepping();
        return;
    }

    if (m_mode == StepForwards)
        m_currentValue += dx;
    else
        m_currentValue -= dx;

    updateUI();
    updateFunctionParameter();
}

void ParameterAnimator::updateFunctionParameter()
{
    m_function->k = m_currentValue;
    View::self()->drawPlot();
}

void ParameterAnimator::gotoInitial()
{
    m_currentValue = m_widget->initial->value();
    updateUI();
    updateFunctionParameter();
}

void ParameterAnimator::gotoFinal()
{
    m_currentValue = m_widget->final->value();
    updateUI();
    updateFunctionParameter();
}

void ParameterAnimator::stepBackwards(bool step)
{
    if (!step) {
        pause();
        return;
    }

    m_mode = StepBackwards;
    startStepping();
    updateUI();
}

void ParameterAnimator::stepForwards(bool step)
{
    if (!step) {
        pause();
        return;
    }

    m_mode = StepForwards;
    startStepping();
    updateUI();
}

void ParameterAnimator::pause()
{
    m_mode = Paused;
    m_timer->stop();
    updateUI();
}

void ParameterAnimator::updateUI()
{
    switch (m_mode) {
    case StepBackwards:
        m_widget->stepBackwards->setChecked(true);
        m_widget->stepForwards->setChecked(false);
        break;

    case StepForwards:
        m_widget->stepBackwards->setChecked(false);
        m_widget->stepForwards->setChecked(true);
        break;

    case Paused:
        m_widget->stepBackwards->setChecked(false);
        m_widget->stepForwards->setChecked(false);
        break;
    }

    m_widget->currentValue->setText(View::self()->posToString(m_currentValue, m_widget->step->value() * 1e-2, View::DecimalFormat));
}

void ParameterAnimator::updateSpeed()
{
    if (m_mode != Paused)
        startStepping();
}

void ParameterAnimator::startStepping() const
{
    double prop = (log(m_widget->speed->value()) - log(m_widget->speed->minimum())) / (log(m_widget->speed->maximum()) - log(m_widget->speed->minimum()));

    // prop = 0  ~ slowest
    // prop = 1  ~ fastest

    int min_ms = 40;
    int max_ms = 1000;

    int ms = int(prop * min_ms + (1 - prop) * max_ms);
    m_timer->start(ms);
}

void ParameterAnimator::stopStepping()
{
    m_timer->stop();
    m_mode = Paused;
    updateUI();
}
// END class ParameterAnimator

#include "moc_parameteranimator.cpp"
