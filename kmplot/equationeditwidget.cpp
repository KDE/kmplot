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

#include "equationeditwidget.h"

#include <QtGui/QFocusEvent>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionFrame>
#include <QtGui/QWheelEvent>

#include "equationedit.h"


void EquationEditWidget::setClearSelectionOnFocusOut(bool doIt) {
	m_clearSelectionOnFocusOut = doIt;
}


EquationEditWidget::EquationEditWidget(EquationEdit* parent) 
	: KTextEdit(parent)
{
	m_clearSelectionOnFocusOut = true;
	m_parent = parent;
	recalculateGeometry();
}


void EquationEditWidget::recalculateGeometry() {
	// Set fixed height
	ensurePolished();
	QFontMetrics fm(document()->defaultFont());
	int h = qMax(fm.lineSpacing(), 14) + 6;
	int m = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	QStyleOptionFrame opt;
	opt.rect = rect();
	opt.palette = palette();
	opt.state = QStyle::State_None;
	setFixedHeight(h + (2 * m));
	
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setContentsMargins(0, 0, 0, 0);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setWordWrapMode(QTextOption::NoWrap);
	setLineWrapMode(KTextEdit::NoWrap);
	setTabChangesFocus(true);
}


void EquationEditWidget::wheelEvent(QWheelEvent* e) {
	e->accept();
}


void EquationEditWidget::keyPressEvent(QKeyEvent* e) {
	if ((e->key() == Qt::Key_Return) || (e->key() == Qt::Key_Enter)) {
		e->accept();
		emit m_parent->editingFinished();
		emit m_parent->returnPressed();
	} else {
		// Still pass these keys to KTextEdit, in case the user has to scroll
		// up/down the text
		if (e->key() == Qt::Key_Up)
			emit m_parent->upPressed();
		else if (e->key() == Qt::Key_Down)
			emit m_parent->downPressed();
		
		KTextEdit::keyPressEvent(e);
	}
}


void EquationEditWidget::focusOutEvent(QFocusEvent* e) {
	KTextEdit::focusOutEvent(e);
	
	if (m_clearSelectionOnFocusOut)
		clearSelection();
	m_parent->reHighlight();
	
	emit m_parent->editingFinished();
}


void EquationEditWidget::focusInEvent(QFocusEvent* e) {
	KTextEdit::focusOutEvent(e);

	m_parent->reHighlight();
	if (e->reason() == Qt::TabFocusReason)
		selectAll();
}


void EquationEditWidget::clearSelection() {
	QTextCursor cursor = textCursor();
	if (!cursor.hasSelection())
		return;
	
	cursor.clearSelection();
	setTextCursor(cursor);
}
