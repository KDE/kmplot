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

#ifndef FktDlg_included
#define FktDlg_included

// locale includes
#include "FktDlgData.h"
#include "AttrDlg.h"
#include "misc.h" 
// Qt includes
#include <qvaluelist.h> 
// KDE includes
#include <kapplication.h>
#include <klocale.h>

class FktDlg : public FktDlgData
{
	Q_OBJECT

public:
	FktDlg( QWidget* parent = NULL, const char* name = NULL );
	virtual ~FktDlg();

protected slots:
	void onok();
	void onclose();
	void ondelete();
	void onedit();
	void ondblclick( int );
	void onattr();
	void onapply();

private:
	int getIx( const QString f_str );
	void updateView();
	int chflg;
	int errflg;
};

#endif // FktDlg_included
