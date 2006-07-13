/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar <f_edemar@linux.se>
*               2006  David Saxton <david@bluehaze.org>
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

#include "kmplotprogress.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kprogressbar.h>

#include <QCoreApplication>
#include <QtGlobal>
#include <QTimer>


KmPlotProgress::KmPlotProgress( QWidget* parent ) : QWidget( parent )
{
	m_button = new KPushButton(this);
	m_button->setIcon( SmallIcon( "cancel" ) );
	m_button->setGeometry( QRect( 0, 0, 30, 23 ) );
	m_button->setMaximumHeight(height()-10);
	connect( m_button, SIGNAL(clicked()), this, SIGNAL(cancelDraw()) );
 
	m_progress = new KProgressBar(this);
	m_progress->setGeometry( QRect( 30, 0, 124, 23 ) );
	m_progress->setMaximumHeight(height()-10);
	
	m_showTimer = new QTimer( this );
	m_showTimer->setSingleShot( true );
	connect( m_showTimer, SIGNAL(timeout()), this, SLOT(showProgressBar()) );
 
	hide();
	setMinimumWidth(154);
}


KmPlotProgress::~KmPlotProgress()
{
}


void KmPlotProgress::showProgressBar()
{
	show();
	QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
}


void KmPlotProgress::setProgress( double progress )
{
	kDebug() << k_funcinfo << "progress="<<progress<<endl;
	
	Q_ASSERT( progress >= 0 );
	
	if ( progress >= 1 )
	{
		hide();
		m_showTimer->stop();
	}
	else
	{
		if ( !isVisible() && !m_showTimer->isActive() )
			m_showTimer->start( 500 );
		
		m_progress->setValue( int(progress * 100) );
	}
	
	QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
}

#include "kmplotprogress.moc"
