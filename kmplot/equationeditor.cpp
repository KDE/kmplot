/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "equationeditor.h"

#include "equationeditorwidget.h"
#include "equationeditwidget.h"
#include <QDialogButtonBox>

EquationEditor::EquationEditor(QWidget *parent)
    : QDialog(parent)
{
    m_widget = new EquationEditorWidget(this);
    m_widget->edit->showEditButton(false);
    m_widget->edit->m_equationEditWidget->setClearSelectionOnFocusOut(false);
    m_widget->layout()->setContentsMargins(0, 0, 0, 0);

    setWindowTitle(i18nc("@title:window", "Equation Editor"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EquationEditor::reject);
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(m_widget);
    dialogLayout->addWidget(buttonBox);

    connect(m_widget->edit, &EquationEdit::returnPressed, this, &EquationEditor::accept);
}

QString EquationEditor::text() const
{
    return m_widget->edit->text();
}

EquationEdit *EquationEditor::edit() const
{
    return m_widget->edit;
}

#include "moc_equationeditor.cpp"
