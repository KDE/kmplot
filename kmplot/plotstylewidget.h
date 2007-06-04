/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2006 David Saxton <david@bluehaze.org>
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
*/

#ifndef PLOTSTYLEWIDGET_H
#define PLOTSTYLEWIDGET_H

#include "function.h"

#include <QGroupBox>

class KColorButton;
class KDialog;
class PlotAppearance;
class PlotStyleDialogWidget;

class PlotStyleWidget : public QGroupBox
{
	Q_OBJECT
	public:
		PlotStyleWidget( QWidget * parent );
		
		/**
		 * Initializes the contents of the widgets to the settings in \p plot.
		 * \p type is used to determine which widgets should be shown / hidden.
		 */
		void init( const PlotAppearance & plot, Function::Type type );
		/**
		 * \return A plot with appearance configured by this widget.
		 * \param visible Whether the plot is visible (this widget does not
		 * control that aspect of a Plot).
		 */
		PlotAppearance plot( bool visible );
		/**
		 * \return the currently selected style.
		 */
		Qt::PenStyle style() const;
		/**
		 * Sets the currently selected style.
		 */
		void setStyle( Qt::PenStyle style );
		
	protected slots:
		/**
		 * Called when the user clicks on the "Advanced" button.
		 */
		void advancedOptions();
		
	protected:
		KColorButton * m_color;
		KDialog * m_dialog;
		PlotStyleDialogWidget * m_dialogWidget;
};

#endif
