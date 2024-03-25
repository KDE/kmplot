/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef KPARAMETEREDITOR_H
#define KPARAMETEREDITOR_H

#include "ui_qparametereditor.h"
#include "xparser.h"

#include <QDialog>
#include <QList>

class QParameterEditor;

/**
@author Fredrik Edemar
*/
/// This class handles the parameter values: it can create, remove, edit and import values.
class KParameterEditor : public QDialog
{
    Q_OBJECT
public:
    KParameterEditor(QList<Value> *, QWidget *parent);
    ~KParameterEditor();

public slots:
    void moveUp();
    void moveDown();
    void prev();
    void next();
    void cmdNew_clicked();
    void cmdDelete_clicked();
    void cmdImport_clicked();
    void cmdExport_clicked();

    /// actions for the visible constant list
    void selectedConstantChanged(QListWidgetItem *current);

    /// updates whether or not the "value is invalid" label is shown, (and returns the validity of the current value)
    bool checkValueValid();

    /// saves the value being edited
    void saveCurrentValue();

    void accept() override;

private:
    /// Check so that it doesn't exist two equal values
    bool checkTwoOfIt(const QString &text);
    QList<Value> *m_parameter;
    QParameterEditor *m_mainWidget;
};

class QParameterEditor : public QWidget, public Ui::QParameterEditor
{
public:
    explicit QParameterEditor(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

#endif
