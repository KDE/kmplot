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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>

#include "kminmax.h"
#include "xparser.h"

KMinMax::KMinMax(QWidget *parent, const char *name)
 : QMinMax(parent, name)
{
}


KMinMax::KMinMax(View *v, QWidget *parent, const char *name)
	: QMinMax(parent, name)
{
	m_view=v;
	m_mode=-1;
	connect( cmdClose, SIGNAL( clicked() ), this, SLOT( close() ));
	connect( cmdFind, SIGNAL( clicked() ), this, SLOT( cmdFind_clicked() ));
	connect( cmdParameter, SIGNAL( clicked() ), this, SLOT( cmdParameter_clicked() ));
	connect( list, SIGNAL( highlighted(QListBoxItem*) ), this, SLOT( list_highlighted(QListBoxItem*) ));
	connect( list, SIGNAL( doubleClicked( QListBoxItem * ) ), this, SLOT( list_doubleClicked(QListBoxItem *) ));
}


void KMinMax::init(char m)
{
	if ( m_mode==m)
	{
		updateFunctions();
		return;
	}
	
	m_mode = m;
	if ( m_mode < 2) //find minimum point
	{
		max->setReadOnly(false);
		min->setText("");
		max->setText("");
		cmdFind->setText("&Find");
		if ( m_mode == 1) //find maximum point
			setCaption(i18n("Find Maximum Point"));
		else
			setCaption(i18n("Find Minimum Point"));
	}
	else if ( m_mode == 2) //get y-value
	{
		setCaption(i18n("Get y-Value"));
		lblMin->setText(i18n("X:"));
		lblMax->setText(i18n("Y:"));	
		max->setReadOnly(true);
		min->setText("");
		max->setText("");
		cmdFind->setText("&Find");

	}
	else if ( m_mode == 3) //area under a graph
	{
		setCaption(i18n("Area Under Graph"));
		lblMin->setText(i18n("Draw the area between the x-values"));
		lblMax->setText(i18n("and"));
		max->setReadOnly(false);
		min->setText("");
		max->setText("");
		cmdFind->setText("&Draw");
	}	

	QString range;
	range.setNum(View::xmin);
	min->setText( range);
	range.setNum(View::xmax);
	max->setText(range);
	
	updateFunctions();
}

void KMinMax::updateFunctions()
{
	list->clear();
	int index;
	QString fname, fstr;
	for ( index = 0; index < m_view->parser()->ufanz; ++index )
	{
		if ( m_view->parser()->getfkt( index, fname, fstr ) == -1 ) continue;
		if( fname[0] != 'x'  && fname[0] != 'y' && fname[0] != 'r')
		{
			if ( m_view->parser()->fktext[ index ].f_mode )
				list->insertItem(m_view->parser()->fktext[ index ].extstr);
			if ( m_view->parser()->fktext[ index ].f1_mode ) //1st derivative
			{
				QString function (m_view->parser()->fktext[ index ].extstr);
				int i= function.find('(');
				function.truncate(i);
				function +="\'";
				list->insertItem(function );
			}
			if ( m_view->parser()->fktext[ index ].f2_mode )//2nd derivative
			{
				QString function (m_view->parser()->fktext[ index ].extstr);
				int i= function.find('(');
				function.truncate(i);
				function +="\'\'";
				list->insertItem(function );
			}
			if ( m_view->parser()->fktext[ index ].anti_mode )//anti derivative
			{
				QString function (m_view->parser()->fktext[ index ].extstr);
				int i= function.find('(');
				function.truncate(i);
				function = function.upper();
				list->insertItem(function );
			}
		}
	}
	selectItem();
}

void KMinMax::selectItem()
{
	cmdParameter->hide();
	parameter="kmplot";
	if (  m_view->csmode < 0)
		return;
	//kdDebug() << "cstype: " << (int)m_view->cstype << endl;
	QString function = m_view->parser()->fktext[ m_view->csmode ].extstr;
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
	
	if (  m_view->parser()->fktext[ m_view->csmode ].k_anz != 0)
		parameter = m_view->parser()->fktext[ m_view->csmode ].str_parameter[m_view->csparam];
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
	if ( m_view->parser()->errmsg()!=0 )
	{
		min->setFocus();
		min->selectAll();
		return;
	}
	if ( m_mode != 2)
	{
		dmax = m_view->parser()->eval(max->text() );
		if ( m_view->parser()->errmsg()!=0 )
		{
			max->setFocus();
			max->selectAll();
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
	bool stop=false;
	int index;
	QString sec_function = function.section('(',0,0);
	for ( index = 0; index < m_view->parser()->ufanz && !stop; ++index )
	{
		if ( m_view->parser()->getfkt( index, fname, fstr ) == -1 ) continue;
		if ( m_view->parser()->fktext[ index ].extstr.section('(',0,0) == sec_function)
			stop=true;
	}
	index--;
	if ( m_view->parser()->fktext[ index ].k_anz == 0)
		parameter = "0";
	else if ( parameter =="kmplot")
	{
		KMessageBox::error(this,i18n("You must choose a parameter for that function"));
		list_highlighted(list->selectedItem() );
		return;
	}
	
	
	if ( m_mode == 0)
	{
		m_view->findMinMaxValue(index,p_mode,true,dmin,dmax,parameter);
		if ( !m_view->isCalculationStopped() )
			KMessageBox::information(this,i18n("Minimum value:\nx: %1\ny: %2").arg(dmin).arg(dmax) );
	}
	else if ( m_mode == 1)
	{
		m_view->findMinMaxValue(index,p_mode,false,dmin,dmax,parameter);
		if ( !m_view->isCalculationStopped() )
			KMessageBox::information(this,i18n("Maximum value:\nx: %1\ny: %2").arg(dmin).arg(dmax));
	}
	else if ( m_mode == 2)
	{
		m_view->getYValue(index,p_mode,dmin,dmax,parameter);
		if ( !m_view->isCalculationStopped() )
		{
			QString tmp;
			tmp.setNum(dmax);
			max->setText(tmp);
		}
	}
	else if ( m_mode == 3)
	{
		double dmin_tmp = dmin;
		m_view->areaUnderGraph(index,p_mode,dmin,dmax,parameter, 0);
		if ( !m_view->isCalculationStopped() )
		{
			m_view->setFocus();
			m_view->update();
			KMessageBox::information(this,i18n("The area between %1 and %1\nis: %3").arg(dmin_tmp).arg(dmax).arg(dmin));
		}
	}
	
	if ( m_view->isCalculationStopped() )
		KMessageBox::error(this,i18n("The operation was cancelled by the user."));
	
	//QDialog::accept();
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
	
	QString fname, fstr;
	bool stop=false;
	int ix;
	QString sec_function = function.section('(',0,0);
	for ( ix = 0; ix < m_view->parser()->ufanz && !stop; ++ix )
	{
		if ( m_view->parser()->getfkt( ix, fname, fstr ) == -1 ) continue;
		if ( m_view->parser()->fktext[ ix ].extstr.section('(',0,0) == sec_function)
			stop=true;
	}
	ix--;
 	if ( m_view->parser()->fktext[ ix ].str_parameter.count() ==0)
 		cmdParameter->hide();
 	else
 		cmdParameter->show();
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
	
	QString fname, fstr;
	bool stop=false;
	int ix;
	QString sec_function = function.section('(',0,0);
	for ( ix = 0; ix < m_view->parser()->ufanz && !stop; ++ix )
	{
		if ( m_view->parser()->getfkt( ix, fname, fstr ) == -1 ) continue;
		if ( m_view->parser()->fktext[ ix ].extstr.section('(',0,0) == sec_function)
			stop=true;
	}
	ix--;

	bool ok;
	QStringList result = KInputDialog::getItemList("Kmplot", i18n("Choose a parameter to use:"), m_view->parser()->fktext[ ix ].str_parameter, QStringList(parameter),false,&ok);
	if ( ok)
		parameter = *result.begin();
}

void KMinMax::list_doubleClicked(QListBoxItem *)
{
 	if ( list->currentItem() == -1)
 		return;
 	else if( cmdParameter->isShown() )
 		cmdParameter_clicked();
}
#include "kminmax.moc"
