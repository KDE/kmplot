/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar <f_edemar@linux.se>
*               2006  David Saxton <david@bluehaze.org>
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

#ifndef FUNCTIONTOOLS_H
#define FUNCTIONTOOLS_H

#include "function.h"

#include <kdialog.h>
#include <QList>
#include <QPair>

class FunctionToolsWidget;
typedef QPair< Plot, int > EquationPair;

/**
@author Fredrik Edemar, David Saxton
*/
/// FunctionTools handles all the dialogs for the items in the tool-menu.
class FunctionTools : public KDialog
{
	Q_OBJECT
	public:
		FunctionTools( QWidget *parent = 0 );
		~FunctionTools();
		
		enum Mode
		{
			FindMinimum,
			FindMaximum,
			CalculateY,
			CalculateArea
		};
		
		/**
		 * Select the right widget to use in the dialog and initalize the plot
		 * list.
		 */
		void init( Mode mode );
		/**
		 * Sets the currently selected equation.
		 */
		void setEquation( const EquationPair & equation );
		/**
		 * \return the currently selected equation.
		 */
		EquationPair equation() const;
		
	protected Q_SLOTS:
		/**
		 * The user selected a different row in the list of plots. Note that
		 * \p equation corresponds to the plot in m_equations.
		 */
		void equationSelected( int equation );
		/**
		 * Called when the min or max range changes.
		 */
		void rangeEdited();
		/**
		 * Called when the x value in the widget changes.
		 */
		void xChanged();
        
	protected:
		/**
		 * Updates the list of equation. This creates the list of equation in
		 * the equations list view and updates m_equations to reflect the
		 * contents of the list view.
		 */
    	void updateEquationList();
		/**
		 * Find the minimum in the current range.
		 */
		void findMinimum( const EquationPair & equation );
		/**
		 * Find the maximum in the current range.
		 */
		void findMaximum( const EquationPair & equation );
		/**
		 * Find the area under the graph
		 */
		void calculateArea( const EquationPair & equation );
		/**
		 * Calculates the value of the selected function for the currently
		 * entered x value.
		 */
		void calculateY( const EquationPair & equation );
	
	private:
		Mode m_mode;
		FunctionToolsWidget * m_widget;
		/**
		 * List of plots and equations.
		 */
		QVector<EquationPair> m_equations;
};

#endif
