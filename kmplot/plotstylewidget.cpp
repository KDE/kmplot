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

#include <klocale.h>

//BEGIN class PlotStyleWidget
PlotStyleWidget::PlotStyleWidget( QWidget * parent )
	: QGroupBox( parent )
{
	setupUi(this);
	
	lineStyle->addItem( i18n("Solid"), Qt::SolidLine );
	lineStyle->addItem( i18n("Dash"), Qt::DashLine );
	lineStyle->addItem( i18n("Dot"), Qt::DotLine );
	lineStyle->addItem( i18n("Dash Dot"), Qt::DashDotLine );
	lineStyle->addItem( i18n("Dash Dot Dot"), Qt::DashDotDotLine );
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
	p.style = style();
	return p;
}


Qt::PenStyle PlotStyleWidget::style( ) const
{
	return (Qt::PenStyle)lineStyle->itemData( lineStyle->currentIndex() ).toInt();
}


void PlotStyleWidget::setStyle( Qt::PenStyle style )
{
	lineStyle->setCurrentIndex( lineStyle->findData( style ) );
}
//END class PlotStyleWidget

#include "plotstylewidget.moc"
