/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2006 David Saxton <david@bluehaze.org>
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
*/

#include "function.h"
#include "plotstylewidget.h"

//BEGIN class PlotStyleWidget
PlotStyleWidget::PlotStyleWidget( QWidget * parent )
	: QGroupBox( parent )
{
	setupUi(this);
}


void PlotStyleWidget::init( const Plot & plot )
{
	color->setColor( plot.color );
	lineWidth->setValue( plot.lineWidth );
}


Plot PlotStyleWidget::plot( bool visible )
{
	Plot p;
	p.color = color->color();
	p.lineWidth = lineWidth->value();
	p.visible = visible;
	return p;
}
//END class PlotStyleWidget

#include "plotstylewidget.moc"
