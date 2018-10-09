/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2006  David Saxton <david@bluehaze.org>
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

#ifndef PARAMETERANIMATOR_H
#define PARAMETERANIMATOR_H

#include <QDialog>

class Function;
class ParameterAnimatorWidget;
class QTimer;

/**
@author David Saxton
 */
class ParameterAnimator : public QDialog
{
	Q_OBJECT
	public:
		ParameterAnimator( QWidget * parent, Function * function );
		~ParameterAnimator();
    
	public slots:
		void gotoInitial();
		void gotoFinal();
		void stepBackwards( bool step );
		void stepForwards( bool step );
		void pause();
		void updateSpeed();
		
	protected slots:
		void step();
		
	protected:
		/**
		 * Start the timer.
		 */
		void startStepping() const;
		/**
		 * Stop the timer, update the buttons.
		 */
		void stopStepping();
		/**
		 * Makes the step buttons toggled / untoggled according to the current
		 * state, and show the current value.
		 */
		void updateUI();
		/**
		 * Gives the current parameter value to the function.
		 */
		void updateFunctionParameter();
		
	private:
		enum AnimateMode
		{
			StepBackwards,
			StepForwards,
			Paused
		};
		
		AnimateMode m_mode;
		double m_currentValue;
		Function * m_function; ///< The function that we're currently animating
		
		ParameterAnimatorWidget *m_widget;
		QTimer * m_timer; ///< for doing the animation
};

#endif
