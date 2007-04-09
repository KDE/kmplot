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

#include "functiontools.h"
#include "ui_functiontools.h"
#include "view.h"
#include "xparser.h"

class FunctionToolsWidget : public QWidget, public Ui::FunctionTools
{
    public:
		FunctionToolsWidget( QWidget * parent = 0 ) : QWidget( parent )
        { setupUi(this); }
};


//BEGIN class FunctionTools
FunctionTools::FunctionTools(QWidget *parent )
	: KDialog( parent )
{
	m_widget = new FunctionToolsWidget( this );
	setMainWidget( m_widget );
	setButtons( Close );
	
	// Adjust margins
	m_widget->layout()->setMargin( 0 );
	
	init( CalculateArea );
	
	connect( m_widget->min, SIGNAL(editingFinished()), this, SLOT(rangeEdited()) );
	connect( m_widget->max, SIGNAL(editingFinished()), this, SLOT(rangeEdited()) );
	connect( m_widget->list, SIGNAL(currentRowChanged(int)), this, SLOT(equationSelected(int)) );
}


FunctionTools::~FunctionTools()
{
}


void FunctionTools::init( Mode m )
{
	m_mode = m;
	
	switch ( m_mode )
	{
		case FindMinimum:
		{
			m_widget->rangeTitle->setText( i18n("Search between:") );
			setCaption(i18n("Find Minimum Point"));
			break;
		}
		
		case FindMaximum:
		{
			m_widget->rangeTitle->setText( i18n("Search between:") );
			setCaption(i18n("Find Maximum Point"));
			break;
		}
		
		case CalculateArea:
		{
			m_widget->rangeTitle->setText( i18n("Calculate the area between:") );
			setCaption(i18n("Area Under Graph"));
			break;
		}
	}
	
	m_widget->min->setText( XParser::self()->number( View::self()->m_xmin ) );
	m_widget->max->setText( XParser::self()->number( View::self()->m_xmax ) );
	m_widget->min->setFocus();
	
	updateEquationList();
	setEquation( EquationPair( View::self()->m_currentPlot, 0 ) );
}


void FunctionTools::updateEquationList()
{
	EquationPair previousEquation = equation();
	
	m_widget->list->clear();
	m_equations.clear();

	foreach ( Function * function, XParser::self()->m_ufkt )
	{
		if ( function->type() != Function::Cartesian && function->type() != Function::Differential )
			continue;
		
		QList<Plot> plots = function->plots();
		
		for ( int i = 0; i < function->eq.size(); ++i )
		{
			foreach ( const Plot &plot, plots )
				m_equations << EquationPair( plot, i );
		}
	}
	
	foreach ( const EquationPair &eq, m_equations )
	{
		Equation * equation = eq.first.function()->eq[ eq.second ];
		QListWidgetItem * item = new QListWidgetItem( equation->fstr(), m_widget->list );
		item->setTextColor( eq.first.color() );
	}
	
	setEquation( previousEquation );
}


EquationPair FunctionTools::equation( ) const
{
	int row = m_widget->list->currentRow();
	if ( row < 0 || row >= m_equations.size() )
		return EquationPair();
	else
		return m_equations[ row ];
}


void FunctionTools::setEquation( const EquationPair & equation )
{
	int row = m_equations.indexOf( equation);
	if ( row < 0 )
		row = 0;
	m_widget->list->setCurrentRow( row );
	equationSelected( row );
}


void FunctionTools::equationSelected( int equation )
{
	if ( equation < 0 || equation >= m_equations.size() )
		return;
	EquationPair current = m_equations[ equation ];
	
	switch ( m_mode )
	{
		case FindMinimum:
			findMinimum( current );
			break;
			
		case FindMaximum:
			findMaximum( current );
			break;
			
		case CalculateArea:
			calculateArea( current );
			break;
	}
}


void FunctionTools::rangeEdited()
{
	switch ( m_mode )
	{
		case FindMinimum:
			findMinimum( equation() );
			break;
			
		case FindMaximum:
			findMaximum( equation() );
			break;
			
		case CalculateArea:
			calculateArea( equation() );
			break;
	}
}


void FunctionTools::findMinimum( const EquationPair & equation )
{
	if ( !equation.first.function() )
		return;
	
	QPointF extremum = View::self()->findMinMaxValue( equation.first, View::Minimum, m_widget->min->value(), m_widget->max->value() );
	
	m_widget->rangeResult->setText( i18n("Minimum is at x = %1, %2(x) = %3",
									extremum.x(),
									equation.first.function()->eq[0]->name(),
									extremum.y() ) );
}


void FunctionTools::findMaximum( const EquationPair & equation )
{
	if ( !equation.first.function() )
		return;
	
	QPointF extremum = View::self()->findMinMaxValue( equation.first, View::Maximum, m_widget->min->value(), m_widget->max->value() );
	
	m_widget->rangeResult->setText( i18n("Maximum is at x = %1, %2(x) = %3",
									extremum.x(),
									equation.first.function()->eq[0]->name(),
									extremum.y() ) );
}


void FunctionTools::calculateArea( const EquationPair & equation )
{
	if ( !equation.first.function() )
		return;
	
	IntegralDrawSettings s;
	s.plot = equation.first;
	s.dmin = m_widget->min->value();
	s.dmax = m_widget->max->value();
                
	double area = View::self()->areaUnderGraph( s );
	
	m_widget->rangeResult->setText( i18n("Area is %1", area ) );
}
//END class FunctionTools


#include "functiontools.moc"
