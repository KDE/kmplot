/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "equationhighlighter.h"

#include <QLocale>

#include "equationedit.h"
#include "equationeditwidget.h"
#include "xparser.h"


EquationHighlighter::EquationHighlighter(EquationEdit* parent)
	: QSyntaxHighlighter(parent->m_equationEditWidget),
	m_parent(parent)
{
	m_errorPosition = -1;
}


EquationHighlighter::~ EquationHighlighter() { }


void EquationHighlighter::highlightBlock(const QString& text) {
	m_parent->checkTextValidity();
	
	if (text.isEmpty())
		return;

    QTextCharFormat number;
    QTextCharFormat function;
    QTextCharFormat variable;
    QTextCharFormat matchedParenthesis;

    QPalette palette;
    if (qGray(palette.color(QPalette::Base).rgb()) > 160) {
        // Good color defaults borrowed from Abakus - thanks! :)
        number.setForeground(QColor(0, 0, 127));
        function.setForeground(QColor(85, 0, 0));
        variable.setForeground(QColor(0, 85, 0));
        matchedParenthesis.setBackground(QColor(255, 255, 183));
    } else {
        number.setForeground(QColor(160, 160, 255));
        function.setForeground(QColor(255, 160, 160));
        variable.setForeground(QColor(160, 255, 160));
        matchedParenthesis.setBackground(QColor(85, 85, 0));
    }
	
	QTextCharFormat other;
	
	const QStringList variables = m_parent->m_equation->variables();
	const QStringList functions = XParser::self()->predefinedFunctions(true) + XParser::self()->userFunctions();
	
	for (int i = 0; i < text.length(); ++i) {
		QString remaining = text.right(text.length() - i);
		
		bool found = false;
		
		for (const QString& var : variables) {
			if (remaining.startsWith(var)) {
				setFormat(i, var.length(), variable);
				i += var.length() - 1;
				found = true;
				break;
			}
		}
		if (found)
			continue;
		
		for (const QString& f : functions) {
			if (remaining.startsWith(f)) {
				setFormat(i, f.length(), function);
				i += f.length() - 1;
				found = true;
				break;
			}
		}
		if (found)
			continue;
		
		ushort u = text[i].unicode();
		bool isFraction = (u >= 0xbc && u <= 0xbe) || (u >= 0x2153 && u <= 0x215e);
		bool isPower = (u >= 0xb2 && u <= 0xb3) || (u == 0x2070) || (u >= 0x2074 && u <= 0x2079);
		bool isDigit = text[i].isDigit();
		bool isDecimalPoint = text[i] == QLocale().decimalPoint();
		
		if (isFraction || isPower || isDigit || isDecimalPoint)
			setFormat(i, 1, number);
		else
			setFormat(i, 1, other);
	}
	
	//BEGIN highlight matched brackets
	int cursorPos = m_parent->m_equationEditWidget->textCursor().position();
	if (cursorPos < 0)
		cursorPos = 0;
	
	// Adjust cursorpos to allow for a bracket before the cursor position
	if (cursorPos >= text.size())
		cursorPos = text.size() - 1;
	else if (cursorPos > 0 && (text[cursorPos-1] == '(' || text[cursorPos-1] == ')'))
		cursorPos--;
	
	bool haveOpen =  text[cursorPos] == '(';
	bool haveClose = text[cursorPos] == ')';
	
	if ((haveOpen || haveClose) && m_parent->hasFocus()) {
		// Search for the other bracket
		
		int inc = haveOpen ? 1 : -1; // which direction to search in
		
		int level = 0;
		for (int i = cursorPos; i >= 0 && i < text.size(); i += inc) {
			if (text[i] == ')')
				level--;
			else if (text[i] == '(')
				level++;
			
			if (level == 0) {
				// Matched!
				setFormat(cursorPos, 1, matchedParenthesis);
				setFormat(i, 1, matchedParenthesis);
				break;
			}
		}
	}
	//END highlight matched brackets
	
	if (m_errorPosition != -1) {
		QTextCharFormat error;
		error.setForeground(Qt::red);
	
		setFormat(m_errorPosition, 1, error);
	}
}


void EquationHighlighter::setErrorPosition(int position) {
	m_errorPosition = position;
}
