/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2005  Fredrik Edemar
*                     f_edemar@linux.se
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

#include <kpopupmenu.h>

#include "sliderwindow.h"

/** @short Slider window for changing a parameter value */
class KSliderWindow : public SliderWindow
{
	Q_OBJECT
	public:
		/// @param parent points to the parent widget.
		/// @param num number of this instance.
		KSliderWindow(QWidget* parent, int num );
		virtual ~KSliderWindow();

	private slots:
		void mnuMinValue_clicked();
		void mnuMaxValue_clicked();

	signals:
		/// emitted when the window has been closed
		void windowClosed(int);

	private:
		bool eventFilter( QObject *obj, QEvent *ev );
		void closeEvent( QCloseEvent * );
		KPopupMenu *m_popupmenu;
		int m_num;
};

#endif
