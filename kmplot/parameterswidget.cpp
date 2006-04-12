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
#include "kparametereditor.h"
#include "parameterswidget.h"
#include "xparser.h"

#include <klocale.h>

//BEGIN class ParametersWidget
ParametersWidget::ParametersWidget( QWidget * parent )
	: QGroupBox( parent )
{
	setupUi(this);
	
	for( int number = 0; number < SLIDER_COUNT; number++ )
		listOfSliders->addItem( i18n( "Slider No. %1", number +1) );
	
	connect( editParameterListButton, SIGNAL(clicked()), this, SLOT(editParameterList()) );
}


void ParametersWidget::init( Function * function )
{
	m_parameters = function->parameters;
	
	if( function->use_slider == -1 )
	{
		listOfSliders->setCurrentIndex( function->use_slider );
		if ( function->parameters.isEmpty() )
			disableParameters->setChecked( true );
		else    
			parametersList->setChecked( true );
	}
	else
	{
		parameterSlider->setChecked( true );
		listOfSliders->setCurrentIndex( function->use_slider );
	}
}


void ParametersWidget::save( Function * function )
{
	function->parameters = m_parameters;
	if ( parameterSlider->isChecked() )
		function->use_slider = listOfSliders->currentIndex(); //specify which slider that will be used
	else
		function->use_slider = -1;
}


void ParametersWidget::editParameterList()
{
	KParameterEditor * dlg = new KParameterEditor( XParser::self(), & m_parameters );
	dlg->exec();
	emit parameterListChanged();
}
//END class ParametersWidget

#include "parameterswidget.moc"
