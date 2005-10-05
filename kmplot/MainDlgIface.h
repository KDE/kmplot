/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar
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

#ifndef MAINDLGIFACE_H
#define MAINDLGIFACE_H

#include <dcopobject.h>

/**
@author Fredrik Edemar
*/
/// All functions in MainDlgIface are accessible with DCOP. For descriptions about the functions, see MainDlg.
class MainDlgIface : virtual public DCOPObject
{
	K_DCOP
k_dcop:
	virtual bool checkModified() = 0;
	
	virtual bool isModified() = 0;
	virtual void editColors() = 0;
	virtual void editAxes() = 0;
	virtual void editScaling() = 0;
	virtual void editFonts() = 0;
	virtual void editConstants() = 0;
	virtual void newFunction() = 0;
	virtual void newParametric() = 0;
	virtual void newPolar() = 0;
	virtual void toggleShowSlider0() = 0;
	virtual void toggleShowSlider1() = 0;
	virtual void toggleShowSlider2() = 0;
	virtual void toggleShowSlider3() = 0;
	virtual void slotSave() = 0;
	virtual void slotSaveas() = 0;
	virtual void slotEditPlots() = 0;
	virtual void slotPrint() = 0;
	virtual void slotExport() = 0;
	virtual void slotSettings() = 0;
	virtual void slotNames() = 0;
	virtual void slotCoord1() = 0;
	virtual void slotCoord2() = 0;
	virtual void slotCoord3() = 0;
	virtual void getYValue() = 0;
	virtual void findMinimumValue() = 0;
	virtual void findMaximumValue() = 0;
	virtual void graphArea() = 0;
};

#endif
