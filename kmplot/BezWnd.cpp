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

//local includes
#include "BezWnd.h"
#include "BezWnd.moc"
#include "MainDlg.h"

#define Inherited BezWndData

BezWnd::BezWnd( QWidget* parent, const char* name ) : Inherited( 0, name )
{
	main_dlg = ( MainDlg * ) parent;
}

BezWnd::~BezWnd()
{
}

void BezWnd::hideEvent( QHideEvent * )
{
	main_dlg->view_bezeichnungen->setChecked( false );
}

