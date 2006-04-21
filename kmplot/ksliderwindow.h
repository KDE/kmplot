/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2005  Fredrik Edemar <f_edemar@linux.se>
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

#include <kmenu.h>

#include "ui_sliderwindow.h"
#include <QEvent>
#include <QCloseEvent>

class KActionCollection;
class SliderWindow;

/** @short Slider window for changing a parameter value */
class KSliderWindow : public KDialog
{
	Q_OBJECT
	public:
		/// @param parent points to the parent widget.
		/// @param ac KActionCollection to add KActions
		KSliderWindow(QWidget* parent, KActionCollection *ac );
		virtual ~KSliderWindow();
		
		int value( int slider );

	private slots:
		void mnuMinValue_clicked();
		void mnuMaxValue_clicked();

	signals:
		/// emitted when the window has been closed
		void windowClosed();
		/// emitted when a slider value changes
		void valueChanged();

	private:
		bool eventFilter( QObject *obj, QEvent *ev );
		void closeEvent( QCloseEvent * );
		void updateMinMaxValues();
		
		KMenu * m_popupmenu;
		QSlider * m_sliders[4];
		QLabel * m_minLabels[4];
		QLabel * m_maxLabels[4];
		SliderWindow * m_mainWidget;
		
		/// this is set to the clicked-on slider when right-clicking on a slider
		QSlider * m_clickedOnSlider;
};


class SliderWindow : public QWidget, public Ui::SliderWindow
{
	public:
		SliderWindow( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};


#endif
