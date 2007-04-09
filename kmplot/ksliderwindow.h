/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2005  Fredrik Edemar <f_edemar@linux.se>
*               2007  David Saxton <david@bluehaze.org>
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

#ifndef KSLIDERWINDOW_H
#define KSLIDERWINDOW_H

#include <KDialog>
#include <QGroupBox>

class SliderWidget;
class QCloseEvent;

/** @short Slider window for changing a parameter value */
class KSliderWindow : public KDialog
{
	Q_OBJECT
	public:
		KSliderWindow( QWidget* parent );
		virtual ~KSliderWindow();
		
		double value( int slider );

	Q_SIGNALS:
		/// emitted when the window has been closed
		void windowClosed();
		/// emitted when a slider value changes
		void valueChanged();

	protected:
		void closeEvent( QCloseEvent * );
		
		SliderWidget * m_sliders[4];
};


#include "ui_sliderwidget.h"
				 
class SliderWidget : public QGroupBox, public Ui::SliderWidget
{
	Q_OBJECT
	public:
		SliderWidget( QWidget *parent, int number );
		~SliderWidget();
		
		double value();
		
	Q_SIGNALS:
		void valueChanged();
		
	protected Q_SLOTS:
		void updateValue();
		
	protected:
		int m_number;
};

#endif
