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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

/** @file FktDlg.h
 * @brief Contains the FktDlg class. Entering and editing functions equation and attributes.
 */

#ifndef FktDlg_included
#define FktDlg_included

// locale includes
#include "FktDlgData.h"

class XParser;

/** This widget class handles the users function input. 
 * Equations can be added, edited, and removed.
 * An attribute settings dialog can be requested.
 */
class FktDlg : public FktDlgData
{
	Q_OBJECT

public:
	/** 
	 * The constructor gets the current parser instance
	 * @param parser current parser instance
	 */
	FktDlg( QWidget* parent, XParser* parser );
	/** And again an empty destructor. */
	virtual ~FktDlg();
	/// fill the widgets with plots contained in th parser instance
	void getPlots();

protected slots:
	/// Delete selected plot
	void slotDelete();
	/// Edit selected Plot
	void slotEdit();
	/// Enables/disables actions if the list has a selection
	void slotHasSelection();
	
	/// edit the plots, index: parser index, num: listbox index
	void slotEditFunction( int index = -1, int num = -1 );
	void slotEditParametric( int x_index = -1, int y_index = -1, int num = -1 );
	void slotEditPolar( int index = -1, int num = -1 );
	void slotNewFunction();
	void slotNewParametric();
	void slotNewPolar();
	void slotHelp();
	
private:
	int getIx( const QString f_str );
	void updateView();
	int chflg;
	int errflg;
	XParser* m_parser;
};

#endif // FktDlg_included
