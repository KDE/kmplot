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

//Added by qt3to4:
#include <QMouseEvent>
#include <QEvent>
#include <QCloseEvent>

// KDE includes
#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

// local includes
#include "ksliderwindow.h"
#include "xparser.h"

KSliderWindow::KSliderWindow( QWidget * parent, KActionCollection * ac ) :
	KDialog( parent, i18n("Sliders") )
{
	m_clickedOnSlider = 0l;
	
	setModal( false );
	m_mainWidget = new SliderWindow( this );
	setMainWidget( m_mainWidget );
	
	m_sliders[0] = m_mainWidget->slider0;
	m_sliders[1] = m_mainWidget->slider1;
	m_sliders[2] = m_mainWidget->slider2;
	m_sliders[3] = m_mainWidget->slider3;
	assert( SLIDER_COUNT == 4 );
	
	KConfig config( "kmplotrc" );
	for ( unsigned i = 0; i < SLIDER_COUNT; ++i )
	{
		m_sliders[i]->setToolTip( i18n( "Slider no. %1" ).arg( i+1 ));
		setWhatsThis( i18n( "Move slider to change the parameter of the function plot connected to this slider." ) );
		
		// load the min and max value + the current value
		config.setGroup( "slider" + QString::number(i) );
		m_sliders[i]->setMinimum( config.readEntry( "min", 0) );
		m_sliders[i]->setMaximum( config.readEntry( "max", 100) );
		m_sliders[i]->setValue( config.readEntry( "value", 50) );
		m_sliders[i]->setPageStep( (int)ceil((abs(m_sliders[i]->minValue()) + abs(m_sliders[i]->maxValue()))/10.) );
		
		m_sliders[i]->installEventFilter(this);
		
		connect( m_sliders[i], SIGNAL( valueChanged( int ) ), this, SIGNAL( valueChanged() ) );
	}
	
	//BEGIN create popup-menu
	m_popupmenu = new KMenu(this);
	
	KAction * mnuMinValue = new KAction( i18n("&Change Minimum Value"), ac, "" );
	connect( mnuMinValue, SIGNAL( triggered(bool) ), this, SLOT( mnuMinValue_clicked() ) );
	mnuMinValue->plug(m_popupmenu);
	
	KAction * mnuMaxValue = new KAction( i18n("&Change Maximum Value"), ac, "" );
	connect( mnuMaxValue, SIGNAL( triggered(bool) ), this, SLOT( mnuMaxValue_clicked() ) );
	mnuMaxValue->plug(m_popupmenu);
	//END create popup-menu
}

KSliderWindow::~KSliderWindow()
{
	// save the min and max value + the current value
	KConfig config( "kmplotrc" );
	
	for ( unsigned i = 0; i < SLIDER_COUNT; ++i )
	{
		config.setGroup( "slider" + QString::number(i) );
		config.writeEntry( "min", m_sliders[i]->minValue() );
		config.writeEntry( "max", m_sliders[i]->maxValue() );
		config.writeEntry( "value", m_sliders[i]->value() );
	}
}


int KSliderWindow::value( int slider )
{
	assert( (slider>=0) && (slider < SLIDER_COUNT) );
	return m_sliders[slider]->value();
}


bool KSliderWindow::eventFilter( QObject *obj, QEvent *ev )
{
	QMouseEvent * mouseEvent = 0l;
	if ( ev->type() == QEvent::MouseButtonPress )
		mouseEvent = static_cast<QMouseEvent*>(ev);
	
	if ( mouseEvent &&
			(mouseEvent->button() == Qt::RightButton) &&
			(obj->metaObject()->className() == "QSlider") )
	{
		m_clickedOnSlider = static_cast<QSlider*>(obj);
		m_popupmenu->exec(QCursor::pos());
		return true;
	}
	
	return KDialog::eventFilter( obj, ev );
}

void KSliderWindow::closeEvent( QCloseEvent * e)
{
	emit windowClosed();
	e->accept();
}

void KSliderWindow::mnuMinValue_clicked()
{
	assert( m_clickedOnSlider );
	
	bool ok;
	int const result = KInputDialog::getInteger(i18n("Change Minimum Value"), i18n("Type a new minimum value for the slider:"), m_clickedOnSlider->minValue(), INT_MIN, INT_MAX, 1, 10, &ok);
	if (!ok)
		return;
	m_clickedOnSlider->setMinimum(result);
	m_clickedOnSlider->setPageStep( (int)ceil((abs(m_clickedOnSlider->maxValue()) + abs(result))/10.) );
	setFocus();
}

void KSliderWindow::mnuMaxValue_clicked()
{
	assert( m_clickedOnSlider );
	
	bool ok;
	int const result = KInputDialog::getInteger(i18n("Change Maximum Value"), i18n("Type a new maximum value for the slider:"), m_clickedOnSlider->maxValue(), INT_MIN, INT_MAX, 1, 10, &ok);
	if (!ok)
		return;
	m_clickedOnSlider->setMaximum(result);
	m_clickedOnSlider->setPageStep( (int)ceil((abs(m_clickedOnSlider->minValue()) + abs(result))/10.) );
	setFocus();
}

#include "ksliderwindow.moc"
