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
#include <qtooltip.h>
#include <qwhatsthis.h> 


#include "kminmax.h"
#include "xparser.h"

KMinMax::KMinMax(View *v, QWidget *parent, const char *name)
	: QMinMax(parent, name), m_view(v)
{
	m_mode=-1;
	connect( cmdClose, SIGNAL( clicked() ), this, SLOT( close() ));
	connect( cmdFind, SIGNAL( clicked() ), this, SLOT( cmdFind_clicked() ));
	connect( cmdParameter, SIGNAL( clicked() ), this, SLOT( cmdParameter_clicked() ));
	connect( list, SIGNAL( highlighted(QListBoxItem*) ), this, SLOT( list_highlighted(QListBoxItem*) ));
	connect( list, SIGNAL( doubleClicked( QListBoxItem * ) ), this, SLOT( list_doubleClicked(QListBoxItem *) ));
	parameter="";
}


void KMinMax::init(char m)
{
	if ( m_mode==m)
	{
		if ( m_mode == 2) //get y-value
			max->setText("");
		updateFunctions();
		return;
	}

	m_mode = m;
	if ( m_mode < 2) //find minimum point
	{
		max->setReadOnly(false);
		QString range;
		range.setNum(View::xmin);
		min->setText( range);
		range.setNum(View::xmax);
		max->setText(range);
		lblMin->setText(i18n("Search between the x-value:"));
		lblMax->setText(i18n("and:"));
		cmdFind->setText(i18n("&Find"));
		QToolTip::add(min,i18n("Lower boundary of the plot range"));
		QWhatsThis::add(min,i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too."));
		QToolTip::add(max,i18n("Upper boundary of the plot range"));
		QWhatsThis::add(max,i18n("Enter the upper boundary of the plot range. Expressions like 2*pi are allowed, too."));
		
		if ( m_mode == 1) //find maximum point
		{
			setCaption(i18n("Find Maximum Point"));
			QToolTip::add( cmdFind, i18n( "Search for the maximum point in the range you specified" ) );
			QWhatsThis::add(cmdFind,i18n("Search for the highest y-value in the x-range you specified and show the result in a message box."));
		}
		else
		{
			setCaption(i18n("Find Minimum Point"));
			QToolTip::add( cmdFind, i18n( "Search for the minimum point in the range you specified" ) );
			QWhatsThis::add(cmdFind,i18n("Search for the lowest y-value in the x-range you specified and show the result in a message box."));
		}
	}
	else if ( m_mode == 2) //get y-value
	{
		setCaption(i18n("Get y-Value"));
		lblMin->setText(i18n("X:"));
		lblMax->setText(i18n("Y:"));
		max->setReadOnly(true);
		min->setText("");
		max->setText("");
		QToolTip::add(min,i18n("Lower boundary of the plot range"));
		QWhatsThis::add(min,i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too."));
		QToolTip::add(max,i18n("No returned y-value yet"));
		QWhatsThis::add(max,i18n("Here you will see the y-value which you got from the x-value in the textbox above. To calculate the y-value, press the Calculate button."));
		
		cmdFind->setText(i18n("&Calculate"));
		QToolTip::add( cmdFind, i18n( "Get the y-value from the x-value you typed" ) );
		QWhatsThis::add(cmdFind,i18n("Get the y-value from the x-value you typed and show it in the y-value box."));

	}
	else if ( m_mode == 3) //area under a graph
	{
		max->setReadOnly(false);
		QString range;
		range.setNum(View::xmin);
		min->setText( range);
		range.setNum(View::xmax);
		max->setText(range);
		QToolTip::add(min,i18n("Lower boundary of the plot range"));
		QWhatsThis::add(min,i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too."));
		QToolTip::add(max,i18n("Upper boundary of the plot range"));
		QWhatsThis::add(max,i18n("Enter the upper boundary of the plot range. Expressions like 2*pi are allowed, too."));
		
		setCaption(i18n("Calculate Integral"));
		lblMin->setText(i18n("Calculate the integral between the x-values:"));
		lblMax->setText(i18n("and:"));
		cmdFind->setText(i18n("&Calculate"));
		QToolTip::add( cmdFind, i18n( "Calculate the integral between the x-values" ) );
		QWhatsThis::add(cmdFind,i18n("Calculate the numeric integral between the x-values and draw the result as an area."));
		
	}

	min->setFocus();
	updateFunctions();
}

void KMinMax::updateFunctions()
{
	QString const selected_item(list->currentText() );
	list->clear();

        for( QValueVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
	{
		if( it->fname[0] != 'x' && it->fname[0] != 'y' && it->fname[0] != 'r' && !it->fname.isEmpty())
		{
			if ( it->f_mode )
				list->insertItem(it->fstr);

			if ( it->f1_mode ) //1st derivative
			{
				QString function (it->fstr);
				int i= function.find('(');
				function.truncate(i);
				function +="\'";
				list->insertItem(function );
			}
			if ( it->f2_mode )//2nd derivative
			{
				QString function (it->fstr);
				int i= function.find('(');
				function.truncate(i);
				function +="\'\'";
				list->insertItem(function );
			}
			if ( it->integral_mode )//integral
			{
				QString function (it->fstr);
				int i= function.find('(');
				function.truncate(i);
				function = function.upper();
				list->insertItem(function );
			}
		}
	}
	list->sort();
	if (list->count()==0) //empty list
		cmdFind->setEnabled(false);
	else
		cmdFind->setEnabled(true);
	selectItem();
	QListBoxItem *found_item = list->findItem(selected_item,Qt::ExactMatch);
	if ( found_item && m_view->csmode < 0)
		list->setSelected(found_item,true);
}

void KMinMax::selectItem()
{
	cmdParameter->hide();
	if (  m_view->csmode < 0)
		return;
	//kdDebug() << "cstype: " << (int)m_view->cstype << endl;
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
	//kdDebug() << "function: " << function << endl;
	QListBoxItem *item = list->findItem(function,Qt::ExactMatch);
	list->setSelected(item,true);

	if (  !ufkt->parameters.isEmpty() )
		parameter = ufkt->parameters[m_view->csparam].expression;
}

KMinMax::~KMinMax()
{
}

void KMinMax::cmdFind_clicked()
{
	if ( list->currentItem() == -1)
	{
		KMessageBox::error(this, i18n("Please choose a function"));
		return;
	}
	double dmin, dmax;
	dmin = m_view->parser()->eval(min->text() );
	if ( m_view->parser()->parserError()!=0 )
	{
		min->setFocus();
		min->selectAll();
		return;
	}
	if ( m_mode != 2)
	{
		dmax = m_view->parser()->eval(max->text() );
		if ( m_view->parser()->parserError()!=0 )
		{
			max->setFocus();
			max->selectAll();
			return;
		}
		if ( dmin >=  dmax)
		{
			KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
			min->setFocus();
			min->selectAll();
			return;
		}

		if (  dmin<View::xmin || dmax>View::xmax )
		{
			KMessageBox::error(this,i18n("Please insert a minimum and maximum range between %1 and %2").arg(View::xmin).arg(View::xmax) );
			min->setFocus();
			min->selectAll();
			return;
		}
	}


	QString function( list->currentText() );
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
		function.at(0) =  function.at(0).lower();
	}

	QString fname, fstr;
	Ufkt *ufkt = 0;
	QString sec_function = function.section('(',0,0);

        for( QValueVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
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
		list_highlighted(list->selectedItem() );
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
			max->setText(tmp);
		}
		QToolTip::add(max,i18n("The returned y-value"));
		QWhatsThis::add(max,i18n("Here you see the result of the calculation: the returned y-value you got from the x-value in the textbox above"));
	}
	else if ( m_mode == 3)
	{
		double dmin_tmp = dmin;
		m_view->areaUnderGraph(ufkt,p_mode,dmin,dmax,parameter, 0);
		if ( !m_view->isCalculationStopped() )
		{
			m_view->setFocus();
			m_view->update();
			KMessageBox::information(this,i18n("The integral in the interval [%1, %2] is:\n%3").arg(dmin_tmp).arg(dmax).arg(dmin));
		}
	}

	if ( m_view->isCalculationStopped() )
		KMessageBox::error(this,i18n("The operation was cancelled by the user."));
}
void KMinMax::list_highlighted(QListBoxItem* item)
{
	if ( !item)
	{
		cmdParameter->hide();
		return;
	}
	QString function( list->currentText() );
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
		function.at(0) =  function.at(0).lower();
	}
	QString const sec_function = function.section('(',0,0);
        for(QValueVector<Ufkt>::iterator it = m_view->parser()->ufkt.begin(); it!=m_view->parser()->ufkt.end(); ++it)
	{
                if ( it->fstr.section('(',0,0) == sec_function)
                {
                        if ( it->parameters.count() == 0)
                                cmdParameter->hide();
                        else
						{
							cmdParameter->show();
							if (parameter.isEmpty() )
								parameter = it->parameters.first().expression;
						}
                        break;
                }
        }
}
void KMinMax::cmdParameter_clicked()
{
	QString function( list->currentText() );
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
		function.at(0) =  function.at(0).lower();
	}
        
	QString const sec_function = function.section('(',0,0);
        for(QValueVector<Ufkt>::iterator it = m_view->parser()->ufkt.begin() ; it!=m_view->parser()->ufkt.end(); ++it)
	{
	       if ( it->fstr.section('(',0,0) == sec_function)
               {
			QStringList str_parameters;
		        for ( QValueList<ParameterValueItem>::Iterator k = it->parameters.begin(); k != it->parameters.end(); ++k )
			       str_parameters.append( (*k).expression);
                        bool ok;
                        QStringList result = KInputDialog::getItemList( i18n("Choose Parameter"), i18n("Choose a parameter to use:"), str_parameters, QStringList(parameter),false,&ok,this );
                        if ( ok)
                                parameter = result.first();
                        break;
                }
	}
}

void KMinMax::list_doubleClicked(QListBoxItem *)
{
 	if ( list->currentItem() == -1)
 		return;
 	else if( cmdParameter->isShown() )
 		cmdParameter_clicked();
}
#include "kminmax.moc"
