/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2005 Fredrik Edemar <f_edemar@linux.se>
    SPDX-FileCopyrightText: 2007  David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef KSLIDERWINDOW_H
#define KSLIDERWINDOW_H

#include <QDialog>
#include <QGroupBox>

class SliderWidget;
class QCloseEvent;

/** @short Slider window for changing a parameter value */
class KSliderWindow : public QDialog
{
    Q_OBJECT
public:
    explicit KSliderWindow(QWidget *parent);
    virtual ~KSliderWindow();

    double value(int slider);

Q_SIGNALS:
    /// emitted when the window has been closed
    void windowClosed();
    /// emitted when a slider value changes
    void valueChanged();

protected:
    void closeEvent(QCloseEvent *) override;

    SliderWidget *m_sliders[4];
};

#include "ui_sliderwidget.h"

class SliderWidget : public QGroupBox, public Ui::SliderWidget
{
    Q_OBJECT
public:
    SliderWidget(QWidget *parent, int number);
    ~SliderWidget();

    double value();

Q_SIGNALS:
    void valueChanged();

protected Q_SLOTS:
    void updateValue();

protected:
    int m_number;
};

#endif
