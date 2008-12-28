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

#include <KAcceleratorManager>


#include "equationeditorwidget.h"
#include "equationeditwidget.h"
#include "maindlg.h"
#include "xparser.h"


EquationEditor::EquationEditor(QWidget* parent)
	: KDialog(parent)
{	
	m_widget = new EquationEditorWidget(this);
	m_widget->edit->showEditButton(false);
	m_widget->edit->m_equationEditWidget->setClearSelectionOnFocusOut(false);
	m_widget->layout()->setMargin(0);
	setMainWidget(m_widget);
	
	setCaption(i18n("Equation Editor"));
	setButtons(Close);
	showButtonSeparator(true);
	
	QFont font;
	double pointSize = font.pointSizeF() * 1.1;
	font.setPointSizeF(pointSize);
	m_widget->edit->m_equationEditWidget->document()->setDefaultFont(font);
	m_widget->edit->m_equationEditWidget->recalculateGeometry();
	
	QFont buttonFont;
	buttonFont.setPointSizeF(font.pointSizeF() * 1.1);
	
	QList<QToolButton*> buttons = m_widget->findChildren<QToolButton*>();
	foreach (QToolButton * w, buttons) {
		KAcceleratorManager::setNoAccel(w);
		
		connect(w, SIGNAL(clicked()), this, SLOT(characterButtonClicked()));
		
		// Also increase the font size, since the fractions, etc are probably not that visible
		// at the default font size
		w->setFont(buttonFont);
	}
	
	connect(m_widget->constantsButton, SIGNAL(clicked()), MainDlg::self(), SLOT(editConstants()));
	connect(m_widget->functionList, SIGNAL(activated(const QString &)), this, SLOT(insertFunction(const QString &)));
	connect(m_widget->constantList, SIGNAL(activated(int)), this, SLOT(insertConstant(int)));
	
	m_widget->functionList->addItems(XParser::self()->predefinedFunctions(false));
	
	connect(m_widget->edit, SIGNAL(returnPressed()), this, SLOT(accept()));
	
	connect(XParser::self()->constants(), SIGNAL(constantsChanged()), this, SLOT(updateConstantList()));
	updateConstantList();
}


void EquationEditor::updateConstantList() {
	QStringList items;
	
	// The first item text is "Insert constant..."
	items << m_widget->constantList->itemText(0);
	
	ConstantList constants = XParser::self()->constants()->list(Constant::All);
	for (ConstantList::iterator it = constants.begin(); it != constants.end(); ++it) {
		QString text = it.key() + " = " + it.value().value.expression();
		items << text;
	}
	
	m_widget->constantList->clear();
	m_widget->constantList->addItems(items);
}


QString EquationEditor::text() const {
	return m_widget->edit->text();
}


EquationEdit* EquationEditor::edit() const {
	return m_widget->edit;
}


void EquationEditor::insertFunction(const QString& function) {
	if (m_widget->functionList->currentIndex() == 0)
		return;

	m_widget->functionList->setCurrentIndex(0);
	m_widget->edit->wrapSelected(function + '(', ")");
	m_widget->edit->setFocus();
}


void EquationEditor::insertConstant(int index) {
	if (index == 0)
		return;

	ConstantList constants = XParser::self()->constants()->list(Constant::All);
	
	if (constants.size() < index)
		return;
	
	// Don't forget that index==0 corresponds to "Insert constant..."
	
	ConstantList::iterator it = constants.begin();
	int at = 0;
	while (++at < index)
		++it;
	
	QString constant = it.key();
	
	m_widget->constantList->setCurrentIndex(0);
	m_widget->edit->insertText(constant);
	m_widget->edit->setFocus();
}


void EquationEditor::characterButtonClicked() {
	const QToolButton* tb = static_cast<const QToolButton*>(sender());
	
	// Something (I can't work out what) is 'helpfully' inserting an ampersand (for keyboard acceleration).
	// Get rid of it.
	m_widget->edit->insertText(tb->text().remove('&'));
}
