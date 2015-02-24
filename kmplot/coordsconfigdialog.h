/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar <f_edemar@linux.se>
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

#ifndef COORDSCONFIGDIALOG_H
#define COORDSCONFIGDIALOG_H

#include <kconfigdialog.h>

namespace Ui{
class EditCoords;
}

class EditCoords;

/**
@author Fredrik Edemar
*/
/// 
class CoordsConfigDialog : public KConfigDialog
{
	Q_OBJECT
	public:
		CoordsConfigDialog(QWidget *parent = 0);
		~CoordsConfigDialog();
		/**
		 * Updates the widgets containing the min/max values.
		 */
		void updateXYRange();
		
	public slots:
		virtual void done(int result) Q_DECL_OVERRIDE;

	protected slots:
		void updateButtons();
		
	private:
		bool evalX(bool showError = true);
		bool evalY(bool showError = true);
		
		EditCoords * configAxesDialog;
};

#endif
