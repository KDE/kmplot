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

#ifndef RstDlg_included
#define RstDlg_included

// local includes
#include "RstDlgData.h"



/**
 * @brief Dialog to set up some grid options.
 */
class RstDlg : public RstDlgData
{
	Q_OBJECT

public:

	RstDlg( QWidget* parent = NULL, const char* name = NULL );
	virtual ~RstDlg();

protected slots:

	void onok();
	void oncancel();

};
#endif // RstDlg_included
