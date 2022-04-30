/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOTSTYLEWIDGET_H
#define PLOTSTYLEWIDGET_H

#include "function.h"

#include <QGroupBox>

class QDialog;
class KColorButton;
class PlotAppearance;
class PlotStyleDialogWidget;

class PlotStyleWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit PlotStyleWidget(QWidget *parent);

    /**
     * Initializes the contents of the widgets to the settings in \p plot.
     * \p type is used to determine which widgets should be shown / hidden.
     */
    void init(const PlotAppearance &plot, Function::Type type);
    /**
     * \return A plot with appearance configured by this widget.
     * \param visible Whether the plot is visible (this widget does not
     * control that aspect of a Plot).
     */
    PlotAppearance plot(bool visible);
    /**
     * \return the currently selected style.
     */
    Qt::PenStyle style() const;
    /**
     * Sets the currently selected style.
     */
    void setStyle(Qt::PenStyle style);

protected slots:
    /**
     * Called when the user clicks on the "Advanced" button.
     */
    void advancedOptions();

protected:
    KColorButton *m_color;
    QDialog *m_dialog;
    PlotStyleDialogWidget *m_dialogWidget;
};

#endif
