/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _PARAMETERSWIDGET_H
#define _PARAMETERSWIDGET_H

#include <QGroupBox>
#include <QList>

#include "function.h"
#include "ui_parameterswidget.h"

class EquationEdit;

class ParametersWidget : public QGroupBox, public Ui_ParametersWidget
{
    Q_OBJECT
public:
    explicit ParametersWidget(QWidget *parent);

    /**
     * Initializes the contents of the widgets to the settings in
     * \p function.
     */
    void init(const ParameterSettings &parameters);
    /**
     * \return the current settings as specified in the widgets.
     */
    ParameterSettings parameterSettings() const;
    /**
     * The ParametersWidget can make sure that when the user wants to use
     * a parameter (i.e. the Use List checkbox or Use Slider checkbox is
     * checked), the function string has a parameter variable. Use this
     * to add an EquationEdit for a function string that ParametersWidget
     * will update when necessary.
     */
    void associateEquationEdit(EquationEdit *edit);

Q_SIGNALS:
    /**
     * Emitted when the user edits the list of parameters.
     */
    void parameterListChanged();

private Q_SLOTS:
    /**
     * Called when the "Edit [parameter] List" button is clicked.
     */
    void editParameterList();
    /**
     * Called when one of the checkboxes is checked.
     */
    void updateEquationEdits();

protected:
    /**
     * If we are currently editing a cartesian function, this will be set
     * to its parameter list.
     */
    QList<Value> m_parameters;
    /**
     * The list of equation edits that may be updated. See sassociateEquationEdit.
     */
    QList<EquationEdit *> m_equationEdits;
};

#endif
