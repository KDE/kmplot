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

#ifndef KOptDlg_included
#define KOptDlg_included

#include "KOptDlgData.h"
#include "misc.h"
#include <kcolordlg.h>
#include <qcheckbox.h>
#include <qlineedit.h>


class KOptDlg : public KOptDlgData
{
	Q_OBJECT

public:

	KOptDlg( QWidget* parent = NULL, const char* name = NULL, bool modal = TRUE );
	virtual ~KOptDlg();

private:

	int ad,
	td,
	tl;
	QColor farbe;

protected slots:

	void onok();
	void oncancel();
	void onfarbe();
};

#endif // KOptDlg_included
