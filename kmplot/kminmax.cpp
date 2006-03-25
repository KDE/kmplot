/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar
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

#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <qlabel.h>
 
//Added by qt3to4:
#include <QList>
#include <q3listview.h>


#include "kminmax.h"
#include "xparser.h"

KMinMax::KMinMax(View *v, QWidget *parent )
	: KDialog( parent, i18n("Find Minimum Point") ),
	  m_view(v)
{
	m_mainWidget = new QMinMax( this );
	setMainWidget( m_mainWidget );
	
	m_mode=-1;
	connect( m_mainWidget->cmdClose, SIGNAL( clicked() ), this, SLOT( close() ));
	connect( m_mainWidget->cmdFind, SIGNAL( clicked() ), this, SLOT( cmdFind_clicked() ));
	connect( m_mainWidget->cmdParameter, SIGNAL( clicked() ), this, SLOT( cmdParameter_clicked() ));
	connect( m_mainWidget->list, SIGNAL( highlighted(Q3ListBoxItem*) ), this, SLOT( list_highlighted(Q3ListBoxItem*) ));
	connect( m_mainWidget->list, SIGNAL( doubleClicked( Q3ListBoxItem * ) ), this, SLOT( list_doubleClicked(Q3ListBoxItem *) ));
	parameter="";
}


void KMinMax::init(char m)
{
	if ( m_mode==m)
	{
		if ( m_mode == 2) //get y-value
			m_mainWidget->max->setText("");
		updateFunctions();
		return;
	}

	m_mode = m;
	if ( m_mode < 2) //find minimum point
	{
		m_mainWidget->max->setReadOnly(false);
		QString range;
		range.setNum(View::xmin);
		m_mainWidget->min->setText( range);
		range.setNum(View::xmax);
		m_mainWidget->max->setText(range);
		m_mainWidget->lblMin->setText(i18n("Search between the x-value:"));
		m_mainWidget->lblMax->setText(i18n("and:"));
		m_mainWidget->cmdFind->setText(i18n("&Find"));
		m_mainWidget->min->setToolTip(i18n("Lower boundary of the plot range"));
		m_mainWidget->min->setWhatsThis(i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too."));
		m_mainWidget->max->setToolTip(i18n("Upper boundary of the plot range"));
		m_mainWidget->max->setWhatsThis(i18n("Enter the upper boundary of the plot range. Expressions like 2*pi are allowed, too."));
		
		if ( m_mode == 1) //find maximum point
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
	}
	else if ( m_mode == 2) //get y-value
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

	}
	else if ( m_mode == 3) //area under a graph
	{
		m_mainWidget->max->setReadOnly(false);
		QString range;
		range.setNum(View::xmin);
		m_mainWidget->min->setText( range);
		range.setNum(View::xmax);
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
		
	}

	m_mainWidget->min->setFocus();
	updateFunctions();
}

void KMinMax::updateFunctions()
{
	QString const selected_item(m_mainWidget->list->currentText() );
	m_mainWidget->list->clear();

        for( QVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
	{
		if( it->fname[0] != 'x' && it->fname[0] != 'y' && it->fname[0] != 'r' && !it->fname.isEmpty())
		{
			if ( it->f_mode )
				m_mainWidget->list->insertItem(it->fstr);

			if ( it->f1_mode ) //1st derivative
			{
				QString function (it->fstr);
				int i= function.find('(');
				function.truncate(i);
				function +="\'";
				m_mainWidget->list->insertItem(function );
			}
			if ( it->f2_mode )//2nd derivative
			{
				QString function (it->fstr);
				int i= function.find('(');
				function.truncate(i);
				function +="\'\'";
				m_mainWidget->list->insertItem(function );
			}
			if ( it->integral_mode )//integral
			{
				QString function (it->fstr);
				int i= function.find('(');
				function.truncate(i);
				function = function.upper();
				m_mainWidget->list->insertItem(function );
			}
		}
	}
	m_mainWidget->list->sort();
	if (m_mainWidget->list->count()==0) //empty m_mainWidget->list
		m_mainWidget->cmdFind->setEnabled(false);
	else
		m_mainWidget->cmdFind->setEnabled(true);
	selectItem();
	Q3ListBoxItem *found_item = m_mainWidget->list->findItem(selected_item,Q3ListView::ExactMatch);
	if ( found_item && m_view->csmode < 0)
		m_mainWidget->list->setSelected(found_item,true);
}

void KMinMax::selectItem()
{
	m_mainWidget->cmdParameter->setEnabled( false );
	if (  m_view->csmode < 0)
		return;
	//kDebug() << "cstype: " << (int)m_view->cstype << endl;
        Ufkt *ufkt = &m_view->parser()->ufkt[m_view->parser()->ixValue(m_view->csmode)];
	QString function = ufkt->fstr;
	if ( m_view->cstype == 2)
	{
		int i= function.find('(');
		function.truncate(i);
		function +="\'\'";
	}
	else if ( m_view->cstype == 1)
	{
		int i= function.find('(');
		function.truncate(i);
		function +="\'";
	}
	//kDebug() << "function: " << function << endl;
	Q3ListBoxItem *item = m_mainWidget->list->findItem(function,Q3ListView::ExactMatch);
	m_mainWidget->list->setSelected(item,true);

	if (  !ufkt->parameters.isEmpty() )
		parameter = ufkt->parameters[m_view->csparam].expression;
}

KMinMax::~KMinMax()
{
}

void KMinMax::cmdFind_clicked()
{
	if ( m_mainWidget->list->currentItem() == -1)
	{
		KMessageBox::error(this, i18n("Please choose a function"));
		return;
	}
	double dmin, dmax;
	dmin = m_view->parser()->eval(m_mainWidget->min->text() );
	if ( m_view->parser()->parserError()!=0 )
	{
		m_mainWidget->min->setFocus();
		m_mainWidget->min->selectAll();
		return;
	}
	if ( m_mode != 2)
	{
		dmax = m_view->parser()->eval(m_mainWidget->max->text() );
		if ( m_view->parser()->parserError()!=0 )
		{
			m_mainWidget->max->setFocus();
			m_mainWidget->max->selectAll();
			return;
		}
		if ( dmin >=  dmax)
		{
			KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
			m_mainWidget->min->setFocus();
			m_mainWidget->min->selectAll();
			return;
		}

		if (  dmin<View::xmin || dmax>View::xmax )
		{
			KMessageBox::error(this,i18n("Please insert a minimum and maximum range between %1 and %2").arg(View::xmin).arg(View::xmax) );
			m_mainWidget->min->setFocus();
			m_mainWidget->min->selectAll();
			return;
		}
	}


	QString function( m_mainWidget->list->currentText() );
	char p_mode = 0;
	if ( function.contains('\'') == 1)
	{
		p_mode = 1;
		int pos = function.find('\'');
		function.remove(pos,1);
	}
	else if ( function.contains('\'') == 2)
	{
		p_mode = 2;
		int pos = function.find('\'');
		function.remove(pos,2);
	}
	else if ( function.at(0).category() == QChar::Letter_Uppercase)
	{
		p_mode = 3;
		function[0] =  function[0].lower();
	}

	QString fname, fstr;
	Ufkt *ufkt = 0;
	QString sec_function = function.section('(',0,0);

        for( QVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
	{
		if ( it->fstr.section('(',0,0) == sec_function)
                {
                        ufkt = it;
			break;
                }
	}
        if ( !ufkt)
        {
                KMessageBox::error(this,i18n("Function could not be found"));
                return;
        }
        
	if ( ufkt->parameters.isEmpty() )
		parameter = "0";
	else if ( parameter.isEmpty())
	{
		KMessageBox::error(this,i18n("You must choose a parameter for that function"));
		list_highlighted(m_mainWidget->list->selectedItem() );
		return;
	}


	if ( m_mode == 0)
	{
		m_view->findMinMaxValue(ufkt,p_mode,true,dmin,dmax,parameter);
		if ( !m_view->isCalculationStopped() )
			KMessageBox::information(this,i18n("Minimum value:\nx: %1\ny: %2").arg(dmin).arg(dmax) );
	}
	else if ( m_mode == 1)
	{
		m_view->findMinMaxValue(ufkt,p_mode,false,dmin,dmax,parameter);
		if ( !m_view->isCalculationStopped() )
			KMessageBox::information(this,i18n("Maximum value:\nx: %1\ny: %2").arg(dmin).arg(dmax));
	}
	else if ( m_mode == 2)
	{
		m_view->getYValue(ufkt,p_mode,dmin,dmax,parameter);
		if ( !m_view->isCalculationStopped() )
		{
			QString tmp;
			tmp.setNum(dmax);
			m_mainWidget->max->setText(tmp);
		}
		m_mainWidget->max->setToolTip(i18n("The returned y-value"));
		m_mainWidget->max->setWhatsThis(i18n("Here you see the result of the calculation: the returned y-value you got from the x-value in the textbox above"));
	}
	else if ( m_mode == 3)
	{
		double dmin_tmp = dmin;
		m_view->areaUnderGraph(ufkt,p_mode,dmin,dmax,parameter, 0);
		if ( !m_view->isCalculationStopped() )
		{
			m_view->setFocus();
			m_view->update();
			KMessageBox::information(this,i18n("The area between %1 and %2\nis: %3").arg(dmin_tmp).arg(dmax).arg(dmin));
		}
	}

	if ( m_view->isCalculationStopped() )
		KMessageBox::error(this,i18n("The operation was cancelled by the user."));
}
void KMinMax::list_highlighted(Q3ListBoxItem* item)
{
	if ( !item)
	{
		m_mainWidget->cmdParameter->setEnabled( false );
		return;
	}
	QString function( m_mainWidget->list->currentText() );
	char p_mode = 0;
	if ( function.contains('\'') == 1)
	{
		p_mode = 1;
		int pos = function.find('\'');
		function.remove(pos,1);
	}
	else if ( function.contains('\'') == 2)
	{
		p_mode = 2;
		int pos = function.find('\'');
		function.remove(pos,2);
	}
	else if ( function.at(0).category() == QChar::Letter_Uppercase)
	{
		p_mode = 3;
		function[0] =  function[0].lower();
	}
	QString const sec_function = function.section('(',0,0);
        for(QVector<Ufkt>::iterator it = m_view->parser()->ufkt.begin(); it!=m_view->parser()->ufkt.end(); ++it)
	{
                if ( it->fstr.section('(',0,0) == sec_function)
                {
                        if ( it->parameters.count() == 0)
							m_mainWidget->cmdParameter->setEnabled( false );
                        else
						{
							m_mainWidget->cmdParameter->setEnabled( true );
							if (parameter.isEmpty() )
								parameter = it->parameters.first().expression;
						}
                        break;
                }
        }
}
void KMinMax::cmdParameter_clicked()
{
	QString function( m_mainWidget->list->currentText() );
	char p_mode = 0;
	if ( function.contains('\'') == 1)
	{
		p_mode = 1;
		int pos = function.find('\'');
		function.remove(pos,1);
	}
	else if ( function.contains('\'') == 2)
	{
		p_mode = 2;
		int pos = function.find('\'');
		function.remove(pos,2);
	}
	else if ( function.at(0).category() == QChar::Letter_Uppercase)
	{
		p_mode = 3;
		function[0] =  function[0].lower();
	}
        
	QString const sec_function = function.section('(',0,0);
        for(QVector<Ufkt>::iterator it = m_view->parser()->ufkt.begin() ; it!=m_view->parser()->ufkt.end(); ++it)
	{
	       if ( it->fstr.section('(',0,0) == sec_function)
               {
			QStringList str_parameters;
		        for ( QList<ParameterValueItem>::Iterator k = it->parameters.begin(); k != it->parameters.end(); ++k )
			       str_parameters.append( (*k).expression);
                        bool ok;
                        QStringList result = KInputDialog::getItemList( i18n("Choose Parameter"), i18n("Choose a parameter to use:"), str_parameters, QStringList(parameter),false,&ok,this );
                        if ( ok)
                                parameter = result.first();
                        break;
                }
	}
}

void KMinMax::list_doubleClicked(Q3ListBoxItem *)
{
 	if ( m_mainWidget->list->currentItem() == -1)
 		return;
	else if( m_mainWidget->cmdParameter->isEnabled() )
 		cmdParameter_clicked();
}
#include "kminmax.moc"
