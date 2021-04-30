/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2007 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QDialog>

class EquationEditorWidget;
class KTextEdit;

/**
 * Mini-calculator for calculating values of functions, etc.
 * @author David Saxton
 */
class Calculator : public QDialog
{
	Q_OBJECT
	public:
		explicit Calculator( QWidget *parent = 0 );
		~Calculator();
		
	protected Q_SLOTS:
		/**
		 * The user hit enter from the input box.
		 */
		void calculate();
        
	protected:
		EquationEditorWidget* m_input;
		KTextEdit* m_display;
		QString m_displayText;
};

#endif
