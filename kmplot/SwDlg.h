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

#ifndef SwDlg_included
#define SwDlg_included

#include "SwDlgData.h"
#include <kapplication.h>
#include <klocale.h>
#include "misc.h"
#include <qlineedit.h>


class SwDlg : public SwDlgData
{
	Q_OBJECT

public:

	SwDlg( QWidget* parent = NULL, const char* name = NULL, bool modal = TRUE );
	virtual ~SwDlg();


protected slots:

	void onok();
	void oncancel();

};

#endif // SwDlg_included
