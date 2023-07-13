/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef FUNCTIONTOOLS_H
#define FUNCTIONTOOLS_H

#include "function.h"

#include <QDialog>
#include <QPair>

class FunctionToolsWidget;
typedef QPair<Plot, int> EquationPair;

/**
@author Fredrik Edemar, David Saxton
*/
/// FunctionTools handles all the dialogs for the items in the tool-menu.
class FunctionTools : public QDialog
{
    Q_OBJECT
public:
    explicit FunctionTools(QWidget *parent = nullptr);
    ~FunctionTools();

    enum Mode { FindMinimum, FindMaximum, CalculateArea };

    /**
     * Select the right widget to use in the dialog and initialize the plot
     * list.
     */
    void init(Mode mode);
    /**
     * Sets the currently selected equation.
     */
    void setEquation(const EquationPair &equation);
    /**
     * \return the currently selected equation.
     */
    EquationPair equation() const;

protected Q_SLOTS:
    /**
     * The user selected a different row in the list of plots. Note that
     * \p equation corresponds to the plot in m_equations.
     */
    void equationSelected(int equation);
    /**
     * Called when the min or max range changes.
     */
    void rangeEdited();

protected:
    /**
     * Updates the list of equation. This creates the list of equation in
     * the equations list view and updates m_equations to reflect the
     * contents of the list view.
     */
    void updateEquationList();
    /**
     * Find the minimum in the current range.
     */
    void findMinimum(const EquationPair &equation);
    /**
     * Find the maximum in the current range.
     */
    void findMaximum(const EquationPair &equation);
    /**
     * Find the area under the graph
     */
    void calculateArea(const EquationPair &equation);

private:
    Mode m_mode;
    FunctionToolsWidget *m_widget;
    /**
     * List of plots and equations.
     */
    QVector<EquationPair> m_equations;
};

#endif
