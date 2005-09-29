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

#include <kiconloader.h>
#include <kprogress.h>

#include "kmplotprogress.h"
//#include "kmplotprogress.moc.cpp"

KmPlotProgress::KmPlotProgress( QWidget* parent, const char* name ) : QWidget( parent, name) 
{
 button = new KPushButton(this);
 button->setPixmap( SmallIcon( "cancel" ) );
 button->setGeometry( QRect( 0, 0, 30, 23 ) );
 button->setMaximumHeight(height()-10);
 
 progress = new KProgress(this);
 progress->setGeometry( QRect( 30, 0, 124, 23 ) );
 progress->setMaximumHeight(height()-10);
 
 hide();
 setMinimumWidth(154);
}

KmPlotProgress::~KmPlotProgress()
{
}

void KmPlotProgress::increase()
{
 progress->setProgress( progress->progress()+1);
}

#include "kmplotprogress.moc"
