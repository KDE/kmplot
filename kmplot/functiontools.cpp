/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "functiontools.h"
#include "ui_functiontools.h"
#include "view.h"
#include "xparser.h"
#include <QDialogButtonBox>

class FunctionToolsWidget : public QWidget, public Ui::FunctionTools
{
    public:
		FunctionToolsWidget( QWidget * parent = 0 ) : QWidget( parent )
        { setupUi(this); }
};


//BEGIN class FunctionTools
FunctionTools::FunctionTools(QWidget *parent )
	: QDialog( parent )
{
	m_widget = new FunctionToolsWidget( this );

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &FunctionTools::reject);
	QVBoxLayout *dialogLayout = new QVBoxLayout (this);
	dialogLayout->addWidget (m_widget);
	dialogLayout->addWidget (buttonBox);
	
	init( CalculateArea );
	
	connect(m_widget->min, &EquationEdit::editingFinished, this, &FunctionTools::rangeEdited);
	connect(m_widget->max, &EquationEdit::editingFinished, this, &FunctionTools::rangeEdited);
	connect(m_widget->list, &QListWidget::currentRowChanged, this, &FunctionTools::equationSelected);
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
			setWindowTitle(i18nc("@title:window", "Find Minimum Point"));
			break;
		}
		
		case FindMaximum:
		{
			m_widget->rangeTitle->setText( i18n("Search between:") );
			setWindowTitle(i18nc("@title:window", "Find Maximum Point"));
			break;
		}
		
		case CalculateArea:
		{
			m_widget->rangeTitle->setText( i18n("Calculate the area between:") );
			setWindowTitle(i18nc("@title:window", "Area Under Graph"));
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

	for ( Function * function : qAsConst(XParser::self()->m_ufkt) )
	{
		if ( function->type() != Function::Cartesian && function->type() != Function::Differential )
			continue;
		
		QList<Plot> plots = function->plots();
		
		for ( int i = 0; i < function->eq.size(); ++i )
		{
			for ( const Plot &plot : qAsConst(plots) )
				m_equations << EquationPair( plot, i );
		}
	}
	
	for ( const EquationPair &eq : qAsConst(m_equations) )
	{
		Equation * equation = eq.first.function()->eq[ eq.second ];
		QListWidgetItem * item = new QListWidgetItem( equation->fstr(), m_widget->list );
		item->setForeground( eq.first.color() );
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
