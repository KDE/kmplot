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

#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <qlabel.h>

#include <QList>

#include "kminmax.h"
#include "ui_qminmax.h"

#include "xparser.h"

class QMinMax : public QWidget, public Ui::QMinMax
{
    public:
        QMinMax( QWidget * parent = 0 )
        { setupUi(this); }
};


KMinMax::KMinMax(QWidget *parent )
	: KDialog( parent )
{
	setCaption( i18n("Find Minimum Point") );

	m_mainWidget = new QMinMax( this );
	setMainWidget( m_mainWidget );

    setCaption( i18n("Find Minimum Point") );
	
	m_mode = CalculateY;
	connect( m_mainWidget->cmdClose, SIGNAL( clicked() ), this, SLOT( close() ));
	connect( m_mainWidget->cmdFind, SIGNAL( clicked() ), this, SLOT( cmdFind_clicked() ));
	connect( m_mainWidget->cmdParameter, SIGNAL( clicked() ), this, SLOT( cmdParameter_clicked() ));
	connect( m_mainWidget->list, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ), this, SLOT( list_currentChanged(QListWidgetItem*) ));
	connect( m_mainWidget->list, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ), this, SLOT( list_doubleClicked(QListWidgetItem *) ));
	
	/// \TODO Use of Parameters needs to be thouroughly checked for KMinMax
}


void KMinMax::init( Mode m )
{
	if ( m_mode == m)
	{
		if ( m_mode == CalculateY )
			m_mainWidget->max->setText("");
		updateFunctions();
		return;
	}

	m_mode = m;
	switch ( m_mode )
	{
		case FindMinimum:
		case FindMaximum:
		{
			m_mainWidget->max->setReadOnly(false);
			QString range;
			range.setNum(View::self()->m_xmin);
			m_mainWidget->min->setText( range);
			range.setNum(View::self()->m_xmax);
			m_mainWidget->max->setText(range);
			m_mainWidget->lblMin->setText(i18n("Search between the x-value:"));
			m_mainWidget->lblMax->setText(i18n("and:"));
			m_mainWidget->cmdFind->setText(i18n("&Find"));
			m_mainWidget->min->setToolTip(i18n("Lower boundary of the plot range"));
			m_mainWidget->min->setWhatsThis(i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too."));
			m_mainWidget->max->setToolTip(i18n("Upper boundary of the plot range"));
			m_mainWidget->max->setWhatsThis(i18n("Enter the upper boundary of the plot range. Expressions like 2*pi are allowed, too."));
		
			if ( m_mode == FindMaximum )
			{
				setCaption(i18n("Find Maximum Point"));
				m_mainWidget->cmdFind->setToolTip( i18n( "Search for the maximum point in the range you specified" ) );
				m_mainWidget->cmdFind->setWhatsThis(i18n("Search for the highest y-value in the x-range you specified and show the result in a message box."));
			}
			else
			{
				setCaption(i18n("Find Minimum Point"));
				m_mainWidget->cmdFind->setToolTip( i18n( "Search for the minimum point in the range you specified" ) );
				m_mainWidget->cmdFind->setWhatsThis(i18n("Search for the lowest y-value in the x-range you specified and show the result in a message box."));
			}
			break;
		}
		
		case CalculateY:
		{
			setCaption(i18n("Get y-Value"));
			m_mainWidget->lblMin->setText(i18n("X:"));
			m_mainWidget->lblMax->setText(i18n("Y:"));
			m_mainWidget->max->setReadOnly(true);
			m_mainWidget->min->setText("");
			m_mainWidget->max->setText("");
			m_mainWidget->min->setToolTip(i18n("Lower boundary of the plot range"));
			m_mainWidget->min->setWhatsThis(i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too."));
			m_mainWidget->max->setToolTip(i18n("No returned y-value yet"));
			m_mainWidget->max->setWhatsThis(i18n("Here you will see the y-value which you got from the x-value in the textbox above. To calculate the y-value, press the Calculate button."));
		
			m_mainWidget->cmdFind->setText(i18n("&Calculate"));
			m_mainWidget->cmdFind->setToolTip( i18n( "Get the y-value from the x-value you typed" ) );
			m_mainWidget->cmdFind->setWhatsThis(i18n("Get the y-value from the x-value you typed and show it in the y-value box."));
			
			break;
		}
		
		case CalculateArea:
		{
			m_mainWidget->max->setReadOnly(false);
			QString range;
			range.setNum(View::self()->m_xmin);
			m_mainWidget->min->setText( range);
			range.setNum(View::self()->m_xmax);
			m_mainWidget->max->setText(range);
			m_mainWidget->min->setToolTip(i18n("Lower boundary of the plot range"));
			m_mainWidget->min->setWhatsThis(i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too."));
			m_mainWidget->max->setToolTip(i18n("Upper boundary of the plot range"));
			m_mainWidget->max->setWhatsThis(i18n("Enter the upper boundary of the plot range. Expressions like 2*pi are allowed, too."));
		
			setCaption(i18n("Area Under Graph"));
			m_mainWidget->lblMin->setText(i18n("Draw the area between the x-values:"));
			m_mainWidget->lblMax->setText(i18n("and:"));
			m_mainWidget->cmdFind->setText(i18n("&Draw"));
			m_mainWidget->cmdFind->setToolTip( i18n( "Draw the area between the function and the y-axis" ) );
			m_mainWidget->cmdFind->setWhatsThis(i18n("Draw the area between the function and the y-axis and show the area in a message box."));
			break;
		}
	}

	m_mainWidget->min->setFocus();
	updateFunctions();
}

void KMinMax::updateFunctions()
{
	QListWidgetItem * currentItem = m_mainWidget->list->currentItem();
	QString const selected_item( currentItem ? currentItem->text() : QString::null );
	
	m_mainWidget->list->clear();

	foreach ( Function * it, XParser::self()->m_ufkt )
	{
		if ( it->type() == Function::Cartesian )
		{
			for ( int i = Function::Derivative0; i <= Function::Integral; ++i )
			{
				if ( it->plotAppearance( (Function::PMode)i ).visible )
					m_mainWidget->list->addItem( it->prettyName( (Function::PMode)i ) );
			}
		}
	}
	m_mainWidget->list->sortItems();
	if (m_mainWidget->list->count()==0) //empty m_mainWidget->list
		m_mainWidget->cmdFind->setEnabled(false);
	else
		m_mainWidget->cmdFind->setEnabled(true);
	selectItem();
	
	QList<QListWidgetItem *> foundItems = m_mainWidget->list->findItems( selected_item, Qt::MatchExactly );
	QListWidgetItem * found_item = foundItems.isEmpty() ? 0 : foundItems.first();
	if ( found_item && View::self()->m_currentPlot.functionID() < 0)
		m_mainWidget->list->setCurrentItem( found_item );
	
	// if there's no item selected, just pick the first one in the list
	if ( ! m_mainWidget->list->currentItem() && (m_mainWidget->list->count() != 0) )
		m_mainWidget->list->setCurrentRow( 0 );
}

void KMinMax::selectItem()
{
	m_mainWidget->cmdParameter->setEnabled( false );
	if ( View::self()->m_currentPlot.functionID() < 0)
		return;
	//kDebug() << "cstype: " << (int)View::self()->cstype << endl;
	Function *ufkt = View::self()->m_currentPlot.function();
	QString function = ufkt->prettyName( View::self()->m_currentPlot.plotMode );
	
	QList<QListWidgetItem *> foundItems = m_mainWidget->list->findItems( function, Qt::MatchExactly );
	if ( !foundItems.isEmpty() )
		m_mainWidget->list->setCurrentItem( foundItems.first() );
	
	parameter = View::self()->m_currentPlot.parameter;
}

KMinMax::~KMinMax()
{
}

void KMinMax::cmdFind_clicked()
{
	if ( !m_mainWidget->list->currentItem() )
	{
		KMessageBox::sorry(this, i18n("Please choose a function"));
		return;
	}
	double dmin = 0.0;
	double dmax = 0.0;
	dmin = XParser::self()->eval(m_mainWidget->min->text() );
	if ( XParser::self()->parserError( true )!=0 )
	{
		m_mainWidget->min->setFocus();
		m_mainWidget->min->selectAll();
		return;
	}
	if ( m_mode != CalculateY )
	{
		dmax = XParser::self()->eval(m_mainWidget->max->text() );
		if ( XParser::self()->parserError( true ) != 0 )
		{
			m_mainWidget->max->setFocus();
			m_mainWidget->max->selectAll();
			return;
		}
		if ( dmin >=  dmax)
		{
			KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
			m_mainWidget->min->setFocus();
			m_mainWidget->min->selectAll();
			return;
		}
	}


	QListWidgetItem * currentItem = m_mainWidget->list->currentItem();
	QString function( currentItem ? currentItem->text() : QString::null );
	Function::PMode p_mode = Function::Derivative0;
	if ( function.count('\'') == 1)
	{
		p_mode = Function::Derivative1;
		int pos = function.indexOf('\'');
		function.remove(pos,1);
	}
	else if ( function.count('\'') == 2)
	{
		p_mode = Function::Derivative2;
		int pos = function.indexOf('\'');
		function.remove(pos,2);
	}
	else if ( function.at(0).category() == QChar::Letter_Uppercase)
	{
		p_mode = Function::Integral;
		function[0] =  function[0].toLower();
	}

	QString fname, fstr;
	Plot plot;
	QString sec_function = function.section('(',0,0);

	foreach ( Function * it, XParser::self()->m_ufkt )
	{
		if ( it->eq[0]->fstr().section('(',0,0) == sec_function)
		{
			plot.setFunctionID( it->id );
			break;
		}
	}
	if ( !plot.functionID() == -1 )
	{
		KMessageBox::sorry(this,i18n("Function could not be found"));
		return;
	}
	
#if 0
	Function * functionPtr = plot.function();
	
	if ( functionPtr->m_parameters.list.isEmpty() )
		parameter = "0";
	else if ( parameter.isEmpty())
	{
		KMessageBox::sorry(this,i18n("You must choose a parameter for that function"));
		QList<QListWidgetItem*> selected = m_mainWidget->list->selectedItems();
		list_currentChanged( selected.isEmpty() ? 0 : selected.first() );
		return;
	}
#endif
	
	plot.parameter = parameter;
	
	switch ( m_mode )
	{
		case FindMinimum:
		{
			QPointF extremum = View::self()->findMinMaxValue( plot, View::Minimum, dmin, dmax );
			if ( !View::self()->isCalculationStopped() )
				KMessageBox::information(this,i18n("Minimum value:\nx: %1\ny: %2", extremum.x(), extremum.y()) );
			break;
		}
		
		case FindMaximum:
		{
			QPointF extremum = View::self()->findMinMaxValue( plot, View::Maximum, dmin, dmax );
			if ( !View::self()->isCalculationStopped() )
				KMessageBox::information(this,i18n("Maximum value:\nx: %1\ny: %2", extremum.x(), extremum.y() ));
			break;
		}
		
		case CalculateY:
		{
			double value = View::self()->value( plot, 0, dmin, true );
			if ( !View::self()->isCalculationStopped() )
			{
				QString tmp;
				tmp.setNum( value );
				m_mainWidget->max->setText(tmp);
			}
			m_mainWidget->max->setToolTip(i18n("The returned y-value"));
			m_mainWidget->max->setWhatsThis(i18n("Here you see the result of the calculation: the returned y-value you got from the x-value in the textbox above"));
			break;
		}
		
		case CalculateArea:
		{
			IntegralDrawSettings s;
			s.plot = plot;
			s.dmin = dmin;
			s.dmax = dmax;
		
			double area = View::self()->areaUnderGraph( s );
			if ( !View::self()->isCalculationStopped() )
			{
				View::self()->setFocus();
				View::self()->update();
				KMessageBox::information(this,i18n("The area between %1 and %2 is\n %3", dmin, dmax, area));
			}
			break;
		}
	}

	if ( View::self()->isCalculationStopped() )
		KMessageBox::sorry(this,i18n("The operation was cancelled by the user."));
}


void KMinMax::list_currentChanged(QListWidgetItem* item)
{
	if ( !item)
	{
		m_mainWidget->cmdParameter->setEnabled( false );
		return;
	}
	QString function( item->text() );
	Function::PMode p_mode = Function::Derivative0;
	if ( function.count('\'') == 1)
	{
		p_mode = Function::Derivative1;
		int pos = function.indexOf('\'');
		function.remove(pos,1);
	}
	else if ( function.count('\'') == 2)
	{
		p_mode = Function::Derivative2;
		int pos = function.indexOf('\'');
		function.remove(pos,2);
	}
	else if ( function.at(0).category() == QChar::Letter_Uppercase)
	{
		p_mode = Function::Integral;
		function[0] =  function[0].toLower();
	}
	QString const sec_function = function.section('(',0,0);
	
	foreach ( Function * it, XParser::self()->m_ufkt )
	{
		if ( it->eq[0]->fstr().section('(',0,0) == sec_function)
		{
			if ( it->m_parameters.list.count() == 0)
				m_mainWidget->cmdParameter->setEnabled( false );
			else
			{
				m_mainWidget->cmdParameter->setEnabled( true );
				parameter.setListPos( 0 );
			}
			break;
		}
	}
}
void KMinMax::cmdParameter_clicked()
{
	QListWidgetItem * currentItem = m_mainWidget->list->currentItem();
	QString function( currentItem ? currentItem->text() : QString::null );
	
	Function::PMode p_mode = Function::Derivative0;
	if ( function.count('\'') == 1)
	{
		p_mode = Function::Derivative1;
		int pos = function.indexOf('\'');
		function.remove(pos,1);
	}
	else if ( function.count('\'') == 2)
	{
		p_mode = Function::Derivative2;
		int pos = function.indexOf('\'');
		function.remove(pos,2);
	}
	else if ( function.at(0).category() == QChar::Letter_Uppercase)
	{
		p_mode = Function::Integral;
		function[0] =  function[0].toLower();
	}
	
#if 0
	QString const sec_function = function.section('(',0,0);
	foreach ( Function * it, XParser::self()->m_ufkt )
	{
		if ( it->eq[0]->fstr().section('(',0,0) == sec_function )
		{
			QStringList str_parameters;
			foreach ( Value v, it->m_parameters.list )
				str_parameters.append( v.expression() );
			bool ok;
			QStringList result = KInputDialog::getItemList( i18n("Choose Parameter"), i18n("Choose a parameter to use:"), str_parameters, QStringList(parameter),false,&ok,this );
			if ( ok )
				parameter.setListPos( str_parameters.indexOf( result.first() ) );
			break;
		}
	}
#endif
}

void KMinMax::list_doubleClicked(QListWidgetItem *)
{
 	if ( !m_mainWidget->list->currentItem() )
 		return;
	else if( m_mainWidget->cmdParameter->isEnabled() )
 		cmdParameter_clicked();
}
#include "kminmax.moc"
