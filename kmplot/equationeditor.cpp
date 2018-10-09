/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C)      2006  David Saxton <david@bluehaze.org>
*               
* This file is part of the KDE Project.
* KmPlot is part of the KDE-EDU Project.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "equationeditor.h"


#include <QDialogButtonBox>
#include "equationeditorwidget.h"
#include "equationeditwidget.h"

EquationEditor::EquationEditor(QWidget* parent)
	: QDialog(parent)
{	
	m_widget = new EquationEditorWidget(this);
	m_widget->edit->showEditButton(false);
	m_widget->edit->m_equationEditWidget->setClearSelectionOnFocusOut(false);
	m_widget->layout()->setMargin(0);

	setWindowTitle(i18n("Equation Editor"));
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &EquationEditor::reject);
	m_widget->layout()->addWidget(buttonBox);

	connect(m_widget->edit, &EquationEdit::returnPressed, this, &EquationEditor::accept);
}


QString EquationEditor::text() const {
	return m_widget->edit->text();
}


EquationEdit* EquationEditor::edit() const {
	return m_widget->edit;
}
