/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2007  David Saxton <david@bluehaze.org>
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

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <KDialog>

class EquationEditorWidget;
class QTextEdit;

/**
 * Mini-calculator for calculating values of functions, etc.
 * @author David Saxton
 */
class Calculator : public KDialog
{
	Q_OBJECT
	public:
		Calculator( QWidget *parent = 0 );
		~Calculator();
		
	protected Q_SLOTS:
		/**
		 * The user hit enter from the input box.
		 */
		void calculate();
        
	protected:
		EquationEditorWidget* m_input;
		QTextEdit *m_display;
		QString m_displayText;
};

#endif
