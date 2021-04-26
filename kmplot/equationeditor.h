/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>
*               
* This file is part of the KDE Project.
* KmPlot is part of the KDE-EDU Project.
*
* SPDX-License-Identifier: GPL-2.0-or-later
*
*/

#ifndef EQUATIONEDITOR_H
#define EQUATIONEDITOR_H

#include <QDialog>

class EquationEdit;
class EquationEditorWidget;


/**
 * A dialog containing an EquationEdit and a variety of buttons to insert
 * special characters.
 * \author David Saxton
 */
class EquationEditor : public QDialog
{
	Q_OBJECT
			
	public:
		explicit EquationEditor(QWidget* parent);
		
		/**
		 * The equation's text, e.g. "f(x) = x^2".
		 */
		QString text() const;
		
		EquationEdit* edit() const;
		
	protected:
		EquationEditorWidget* m_widget;
};

#endif
