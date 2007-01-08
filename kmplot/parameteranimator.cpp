/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2006  David Saxton <david@bluehaze.org>
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

#include "parameteranimator.h"
#include "ui_parameteranimator.h"
#include "view.h"

#include <kicon.h>
#include <QTimer>

#include <assert.h>
#include <cmath>
using namespace std;

class ParameterAnimatorWidget : public QWidget, public Ui::ParameterAnimator
{
	public:
		ParameterAnimatorWidget( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};



//BEGIN class ParameterAnimator
ParameterAnimator::ParameterAnimator( QWidget * parent, Function * function )
	: KDialog( parent ),
	m_function( function )
{
	m_widget = new ParameterAnimatorWidget( this );
	m_widget->layout()->setMargin( 0 );
	setMainWidget( m_widget );
	
	setCaption( i18n("Parameter Animator") );
	setButtons( Close );
	
	m_mode = Paused;
	m_currentValue = 0;
	m_function->m_parameters.animating = true;
	m_function->k = m_currentValue;
	
	if ( function->eq[0]->usesParameter() )
		m_widget->warningLabel->hide();
	
	m_timer = new QTimer( this );
	connect( m_timer, SIGNAL(timeout()), this, SLOT(step()) );
	
	m_widget->gotoInitial->setIcon( KIcon( "2leftarrow" ) );
	m_widget->gotoFinal->setIcon( KIcon( "2rightarrow" ) );
	m_widget->stepBackwards->setIcon( KIcon( "1leftarrow" ) );
	m_widget->stepForwards->setIcon( KIcon( "1rightarrow" ) );
	m_widget->pause->setIcon( KIcon( "player_pause" ) );
	
	connect( m_widget->gotoInitial, SIGNAL(clicked()), this, SLOT(gotoInitial()) );
	connect( m_widget->gotoFinal, SIGNAL(clicked()), this, SLOT(gotoFinal()) );
	connect( m_widget->stepBackwards, SIGNAL(toggled(bool)), this, SLOT(stepBackwards(bool)) );
	connect( m_widget->stepForwards, SIGNAL(toggled(bool)), this, SLOT(stepForwards(bool)) );
	connect( m_widget->pause, SIGNAL(clicked()), this, SLOT(pause()) );
	connect( m_widget->speed, SIGNAL(valueChanged(int)), this, SLOT(updateSpeed()) );
	
	updateUI();
	updateFunctionParameter();
	
	connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}


ParameterAnimator::~ ParameterAnimator()
{
	kDebug() << k_funcinfo << endl;
	m_function->m_parameters.animating = false;
	View::self()->drawPlot();
}


void ParameterAnimator::step()
{
	// This function shouldn't get called when we aren't actually stepping
	assert( m_mode != Paused );
	
	double dx = m_widget->step->value();
	
	bool increasing = ( (m_mode == StepBackwards && (dx < 0)) || (m_mode == StepForwards && (dx > 0)) );
	bool decreasing = ( (m_mode == StepBackwards && (dx > 0)) || (m_mode == StepForwards && (dx < 0)) );
	
	double upper = m_widget->final->value();
	double lower = m_widget->initial->value();
	
	if ( lower > upper )
		qSwap( lower, upper );
	
	if ( (increasing && (m_currentValue >= upper)) ||
			 (decreasing && (m_currentValue <= lower)) )
	{
		stopStepping();
		return;
	}
	
	if ( m_mode == StepForwards )
		m_currentValue += dx;
	else
		m_currentValue -= dx;
	
	updateUI();
	updateFunctionParameter();
}


void ParameterAnimator::updateFunctionParameter()
{
	m_function->k = m_currentValue;
	View::self()->drawPlot();
}


void ParameterAnimator::gotoInitial()
{
	m_currentValue = m_widget->initial->value();
	updateUI();
	updateFunctionParameter();
}


void ParameterAnimator::gotoFinal()
{
	m_currentValue = m_widget->final->value();
	updateUI();
	updateFunctionParameter();
}


void ParameterAnimator::stepBackwards( bool step )
{
	if ( !step )
	{
		pause();
		return;
	}
	
	m_mode = StepBackwards;
	startStepping();
	updateUI();
}


void ParameterAnimator::stepForwards( bool step )
{
	if ( !step )
	{
		pause();
		return;
	}
	
	m_mode = StepForwards;
	startStepping();
	updateUI();
}


void ParameterAnimator::pause()
{
	m_mode = Paused;
	m_timer->stop();
	updateUI();
}


void ParameterAnimator::updateUI()
{
	switch ( m_mode )
	{
		case StepBackwards:
			m_widget->stepBackwards->setChecked( true );
			m_widget->stepForwards->setChecked( false );
			break;
			
		case StepForwards:
			m_widget->stepBackwards->setChecked( false );
			m_widget->stepForwards->setChecked( true );
			break;
			
		case Paused:
			m_widget->stepBackwards->setChecked( false );
			m_widget->stepForwards->setChecked( false );
			break;
	}
	
	m_widget->currentValue->setText( View::self()->posToString( m_currentValue, m_widget->step->value() * 1e-2, View::DecimalFormat ) );
}


void ParameterAnimator::updateSpeed()
{
	if ( m_mode != Paused )
		startStepping();
}


void ParameterAnimator::startStepping() const
{
	double prop = (log( m_widget->speed->value() ) - log( m_widget->speed->minimum() )) / (log( m_widget->speed->maximum()) - log( m_widget->speed->minimum()));
	
	// prop = 0  ~ slowest
	// prop = 1  ~ fastest
	
	int min_ms = 40;
	int max_ms = 1000;
	
	int ms = int( prop*min_ms + (1-prop)*max_ms );
	m_timer->start( ms );
}


void ParameterAnimator::stopStepping()
{
	m_timer->stop();
	m_mode = Paused;
	updateUI();
}
//END class ParameterAnimator

#include "parameteranimator.moc"