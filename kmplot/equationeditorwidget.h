/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef EQUATIONEDITORWIDGET_H
#define EQUATIONEDITORWIDGET_H

#include "ui_equationeditorwidget.h"

class EquationEditorWidget : public QWidget, public Ui::EquationEditorWidget
{
    Q_OBJECT

public:
    explicit EquationEditorWidget(QWidget *parent = 0);

protected slots:
    void insertFunction(const QString &text);
    /**
     * \p index is the position of the constant in the constants combo box.
     */
    void insertConstant(int index);
    void characterButtonClicked();
    void updateConstantList();
    void editConstants();
};

#endif
