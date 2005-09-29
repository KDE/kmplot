/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2005  Fredrik Edemar
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
// Qt includes
#include <qcursor.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// KDE includes
#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>

#include <limits.h>
#include <math.h>
#include <stdlib.h>

// local includes
#include "ksliderwindow.h"

KSliderWindow::KSliderWindow(QWidget* parent, int num ) :
	SliderWindow( parent, "", false, Qt::WStyle_Tool-Qt::WStyle_Maximize ), m_num(num)
{
	setCaption(i18n( "Slider %1" ).arg( num+1 ) );
	QToolTip::add( slider, i18n( "Slider no. %1" ).arg( num+1 ));
	QWhatsThis::add( this, i18n( "Move slider to change the parameter of the function plot connected to this slider." ) );
	
	// load the min and max value + the current value
	KConfig config( "kmplotrc" );
	config.setGroup( "slider" + QString::number(num) );
	slider->setMinValue( config.readNumEntry( "min", 0) );
	slider->setMaxValue( config.readNumEntry( "max", 100) );
	slider->setValue( config.readNumEntry( "value", 50) );
	slider->setPageStep( (int)ceil((abs(slider->minValue()) + abs(slider->maxValue()))/10.) );
	
	slider->installEventFilter(this);
	installEventFilter(this);
	
	m_popupmenu = new KPopupMenu(this);
	KAction *mnuMinValue = new KAction(i18n("&Change Minimum Value") ,0,this, SLOT( mnuMinValue_clicked() ),0);
	mnuMinValue->plug(m_popupmenu);
	KAction *mnuMaxValue = new KAction(i18n("&Change Maximum Value") ,0,this, SLOT( mnuMaxValue_clicked() ),0 );
	mnuMaxValue->plug(m_popupmenu);
}

KSliderWindow::~KSliderWindow()
{
	// save the min and max value + the current value
	KConfig config( "kmplotrc" );
	config.setGroup( "slider" + QString::number(m_num) );
	config.writeEntry( "min", slider->minValue() );
	config.writeEntry( "max", slider->maxValue() );
	config.writeEntry( "value", slider->value() );
}

bool KSliderWindow::eventFilter( QObject *obj, QEvent *ev )
{
	if (ev->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent *e = (QMouseEvent *)ev;
 		if (e->button() != Qt::RightButton)
			return SliderWindow::eventFilter( obj, ev );
		m_popupmenu->exec(QCursor::pos());
		return true;
	}
	return SliderWindow::eventFilter( obj, ev );
}

void KSliderWindow::closeEvent( QCloseEvent * e)
{
	emit windowClosed(m_num);
	e->accept();
}

void KSliderWindow::mnuMinValue_clicked()
{
	bool ok;
	int const result = KInputDialog::getInteger(i18n("Change Minimum Value"), i18n("Type a new minimum value for the slider:"), slider->minValue(), INT_MIN, INT_MAX, 1, 10, &ok);
	if (!ok)
		return;
	slider->setMinValue(result);
	slider->setPageStep( (int)ceil((abs(slider->maxValue()) + abs(result))/10.) );
	setFocus();
}

void KSliderWindow::mnuMaxValue_clicked()
{
	bool ok;
	int const result = KInputDialog::getInteger(i18n("Change Maximum Value"), i18n("Type a new maximum value for the slider:"), slider->maxValue(), INT_MIN, INT_MAX, 1, 10, &ok);
	if (!ok)
		return;
	slider->setMaxValue(result);
	slider->setPageStep( (int)ceil((abs(slider->minValue()) + abs(result))/10.) );
	setFocus();
}

#include "ksliderwindow.moc"
