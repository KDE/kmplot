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

#ifndef EQUATIONEDITOR_H
#define EQUATIONEDITOR_H

#include <KDialog>

class EquationEdit;
class EquationEditorWidget;


/**
 * A dialog containing an EquationEdit and a variety of buttons to insert
 * special characters.
 * \author David Saxton
 */
class EquationEditor : public KDialog
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
