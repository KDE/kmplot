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

// Qt includes
#include <qvaluelist.h> 

// KDE includes
#include <kapplication.h>
#include <klocale.h>

// locale includes
#include "FktDlgData.h"
#include "misc.h" 

class KEditFunction;

/** This widget class handles the users function input. 
 * Equations can be added, edited, and removed.
 * An attribute settings dialog can be requested.
 */
class FktDlg : public FktDlgData
{
	Q_OBJECT

public:
	/** 
	 * The constructor fills the GUI widgets with the content 
	 * of the parsers members.
	 *
	 * If the parser contains no functions, that is especially if this dialog is
	 * opened for the first time, "f(x)=" is suggested in the edit line.
	 */
	FktDlg( QWidget* parent = NULL, const char* name = NULL );
	/** And again an empty destructor. */
	virtual ~FktDlg();
	
	void fillList();

protected slots:
	void ondelete();
	void onedit();
	// void onnew();
	void onHasSelection();
	
	// edit the function, index: parser index, num: listbox index
	void onEditFunction( int index = -1, int num = -1 );
	void onEditParametric( int x_index = -1, int y_index = -1, int num = -1 );
	void onEditPolar( int index = -1, int num = -1 );
	void onNewFunction();
	void onNewParametric();
	void onNewPolar();

private:
	int getIx( const QString f_str );
	void updateView();
	int chflg;
	int errflg;
	KEditFunction *editFunction;
};

#endif // FktDlg_included
