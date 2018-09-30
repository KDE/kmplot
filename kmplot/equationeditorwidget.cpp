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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#include "equationeditorwidget.h"

#include <KAcceleratorManager>

#include "equationeditwidget.h"
#include "maindlg.h"
#include "xparser.h"


EquationEditorWidget::EquationEditorWidget(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
	
	QFont font;
	double pointSize = font.pointSizeF() * 1.1;
	font.setPointSizeF(pointSize);
	edit->m_equationEditWidget->document()->setDefaultFont(font);
	edit->m_equationEditWidget->recalculateGeometry();
	
	QFont buttonFont;
	buttonFont.setPointSizeF(font.pointSizeF() * 1.1);
	
	QList<QToolButton*> buttons = findChildren<QToolButton*>();
	foreach (QToolButton* w, buttons) {
		KAcceleratorManager::setNoAccel(w);
		
		connect(w, SIGNAL(clicked()), this, SLOT(characterButtonClicked()));
		
		// Also increase the font size, since the fractions, etc are probably not that visible
		// at the default font size
		w->setFont(buttonFont);
	}
	
	connect(constantsButton, SIGNAL(clicked()), this, SLOT(editConstants()));
	connect(functionList, SIGNAL(activated(QString)), this, SLOT(insertFunction(QString)));
	connect(constantList, SIGNAL(activated(int)), this, SLOT(insertConstant(int)));
	
	QStringList functions = XParser::self()->predefinedFunctions(false);
	functions.sort();
	functionList->addItems(functions);
	
	connect(XParser::self()->constants(), SIGNAL(constantsChanged()), this, SLOT(updateConstantList()));
	updateConstantList();
}


void EquationEditorWidget::updateConstantList() {
	QStringList items;
	
	// The first item text is "Insert constant..."
	items << constantList->itemText(0);
	
	ConstantList constants = XParser::self()->constants()->list(Constant::All);
	for (ConstantList::iterator it = constants.begin(); it != constants.end(); ++it) {
		QString text = it.key() + " = " + it.value().value.expression();
		items << text;
	}
	
	constantList->clear();
	constantList->addItems(items);
}


void EquationEditorWidget::insertFunction(const QString& function) {
	if (functionList->currentIndex() == 0)
		return;

	functionList->setCurrentIndex(0);
	edit->wrapSelected(function + '(', ")");
	edit->setFocus();
}


void EquationEditorWidget::editConstants()
{
	MainDlg::self()->editConstantsModal(this);
}


void EquationEditorWidget::insertConstant(int index) {
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
	
	constantList->setCurrentIndex(0);
	edit->insertText(constant);
	edit->setFocus();
}


void EquationEditorWidget::characterButtonClicked() {
	const QToolButton* tb = static_cast<const QToolButton*>(sender());
	
	// Something (I can't work out what) is 'helpfully' inserting an ampersand (for keyboard acceleration).
	// Get rid of it.
	edit->insertText(tb->text().remove('&'));
}
