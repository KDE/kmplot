/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef PARAMETERANIMATOR_H
#define PARAMETERANIMATOR_H

#include <QDialog>

class Function;
class ParameterAnimatorWidget;
class QTimer;

/**
@author David Saxton
 */
class ParameterAnimator : public QDialog
{
    Q_OBJECT
public:
    ParameterAnimator(QWidget *parent, Function *function);
    ~ParameterAnimator();

public slots:
    void gotoInitial();
    void gotoFinal();
    void stepBackwards(bool step);
    void stepForwards(bool step);
    void pause();
    void updateSpeed();

protected slots:
    void step();

protected:
    /**
     * Start the timer.
     */
    void startStepping() const;
    /**
     * Stop the timer, update the buttons.
     */
    void stopStepping();
    /**
     * Makes the step buttons toggled / untoggled according to the current
     * state, and show the current value.
     */
    void updateUI();
    /**
     * Gives the current parameter value to the function.
     */
    void updateFunctionParameter();

private:
    enum AnimateMode { StepBackwards, StepForwards, Paused };

    AnimateMode m_mode;
    double m_currentValue;
    Function *m_function; ///< The function that we're currently animating

    ParameterAnimatorWidget *m_widget;
    QTimer *m_timer; ///< for doing the animation
};

#endif
