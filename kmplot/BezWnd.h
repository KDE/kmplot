/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
*               2000, 2002 kd.moeller@t-online.de
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
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*/

/** @file BezWnd.h
 * @brief Showing predefined function names and constants (BezWnd).
 */

#ifndef BezWnd_included
#define BezWnd_included

#include "BezWndData.h"

class MainDlg;

/**
 * This Widget shows a list of predefined functions and constants of the implemented math parser.
 * Its connected to a KToggleAction verw_bezeichnungen of the main widget.
 * @see hideEvent, MainDlg::view_bezeichnungen
 */
class BezWnd : public BezWndData
{
	Q_OBJECT

public:
	/** Common constructor. 
	 * It only sets the main_dlg to parent.
	 */
	BezWnd( QWidget* parent = NULL, const char* name = NULL );
	/** Empty destructor. */
	virtual ~BezWnd();

protected slots:
	/** Reimplemented. 
	 * Sets the KToggleAction view_bezeichnungen to false.
	 * @see hideEvent, MainDlg::view_bezeichnungen
	 */
void hideEvent( QHideEvent * );

private:
	/** A Pointer to the central class instance */
	MainDlg *main_dlg;
};
#endif // BezWnd_included
