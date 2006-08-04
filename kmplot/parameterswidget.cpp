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
	connect( useSlider,	SIGNAL(toggled(bool)), this, SLOT(updateEquationEdits()) );
	connect( useList,	SIGNAL(toggled(bool)), this, SLOT(updateEquationEdits()) );
}


void ParametersWidget::init( const ParameterSettings & settings )
{
	useSlider->setChecked( settings.useSlider );
	useList->setChecked( settings.useList );
	
	listOfSliders->setCurrentIndex( settings.sliderID );
	m_parameters = settings.list;
}


ParameterSettings ParametersWidget::parameterSettings() const
{
	ParameterSettings s;
	
	s.useSlider = useSlider->isChecked();
	s.useList = useList->isChecked();
	
	s.sliderID = listOfSliders->currentIndex();
	s.list = m_parameters;
	
	return s;
}


void ParametersWidget::editParameterList()
{
	KParameterEditor * dlg = new KParameterEditor( & m_parameters );
	dlg->exec();
	emit parameterListChanged();
}


void ParametersWidget::updateEquationEdits( )
{
	if ( !useSlider->isChecked() && !useList->isChecked() )
	{
		// Don't need to add any parameter variables
		return;
	}
	
	foreach ( EquationEdit * edit, m_equationEdits )
	{
		if ( edit->equation()->usesParameter() || !edit->equation()->looksLikeFunction() )
			continue;
		
		QString text = edit->text();
		int bracket = text.indexOf( ')' );
		if ( bracket < 0 )
			continue;
		
		text.replace( bracket, 1, ",k)" );
		edit->setText( text );
	}
}


void ParametersWidget::associateEquationEdit( EquationEdit * edit )
{
	m_equationEdits << edit;
}
//END class ParametersWidget

#include "parameterswidget.moc"
