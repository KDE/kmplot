/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
*               2000, 2002 kd.moeller@t-online.de
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

// Qt includes
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qvbox.h>


// KDE includes
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kpushbutton.h>

#include <kdebug.h>

// local includes
#include "editfunction.h"
#include "editfunction.moc"
#include "View.h"
#include "editfunctionpage.h"
#include "editderivativespage.h"
#include "editintegralpage.h"
#include "kparametereditor.h"

EditFunction::EditFunction( XParser* parser, QWidget* parent, const char* name ) : 
	KDialogBase( IconList, "Caption", Help|Ok|Cancel, Ok, parent, name ), m_parser(parser)
{
	QVBox *page0 = addVBoxPage( i18n("Function"), i18n( "Function" ), SmallIcon( "func", 32 ) );
	editfunctionpage = new EditFunctionPage( page0 );
	QVBox *page1 = addVBoxPage( i18n("Derivatives"), i18n( "Derivatives" ), SmallIcon( "deriv_func", 32 ) );
	editderivativespage = new EditDerivativesPage( page1 );
	QVBox *page2 = addVBoxPage( i18n("Integral"), i18n( "Integral" ), SmallIcon( "integral_func", 32 ) );
	editintegralpage = new EditIntegralPage( page2 );
	for( int number = 0; number < SLIDER_COUNT; number++ )
	{
		editfunctionpage->listOfSliders->insertItem( i18n( "Slider No. %1" ).arg( number +1) );
	}
	connect( editfunctionpage->cmdParameter, SIGNAL ( clicked() ), this, SLOT( cmdParameter_clicked() ) );
	connect( editfunctionpage->useNoParameter, SIGNAL ( toggled(bool) ), this, SLOT( noParameter_toggled(bool) ) );
	connect( editfunctionpage->customMinRange, SIGNAL ( toggled(bool) ), this, SLOT( customMinRange_toggled(bool) ) );
	connect( editfunctionpage->customMaxRange, SIGNAL ( toggled(bool) ), this, SLOT( customMaxRange_toggled(bool) ) );
    m_updatedfunction = 0;
}

void EditFunction::initDialog( int id )
{
	m_id = id;
	if( m_id == -1 ) clearWidgets(); //new function, so clear all values
	else setWidgets();
	editfunctionpage->equation->setFocus();
}

void EditFunction::clearWidgets()
{
	// Clear the Function page
	editfunctionpage->equation->clear();
	editfunctionpage->hide->setChecked( false );
	editfunctionpage->customMinRange->setChecked( false );
	editfunctionpage->customMaxRange->setChecked( false );
	editfunctionpage->min->clear();
	editfunctionpage->max->clear();
	editfunctionpage->lineWidth->setValue( m_parser->linewidth0 );
	editfunctionpage->color->setColor( m_parser->defaultColor(m_parser->getNextIndex() ) );

	// Clear the Derivatives page
	editderivativespage->showDerivative1->setChecked( false );
	editderivativespage->lineWidthDerivative1->setValue( editfunctionpage->lineWidth->value() );
	editderivativespage->colorDerivative1->setColor( editfunctionpage->color->color() );
	editderivativespage->showDerivative2->setChecked( false );
	editderivativespage->lineWidthDerivative2->setValue( editfunctionpage->lineWidth->value() );
	editderivativespage->colorDerivative2->setColor( editfunctionpage->color->color() );

	// Clear the Integral page
	editintegralpage->precision->setValue( Settings::stepWidth());
	editintegralpage->color->setColor( editfunctionpage->color->color() );
	editintegralpage->lineWidth->setValue(editfunctionpage->lineWidth->value());
	
}

void EditFunction::setWidgets()
{
	Ufkt *ufkt = &m_parser->ufkt[ m_parser->ixValue(m_id) ];
	editfunctionpage->equation->setText( ufkt->fstr );
	editfunctionpage->hide->setChecked( !ufkt->f_mode);
	editfunctionpage->lineWidth->setValue( ufkt->linewidth );
	editfunctionpage->color->setColor( ufkt->color );
	
	if (ufkt->usecustomxmin)
	{
	  editfunctionpage->customMinRange->setChecked(true);
	  editfunctionpage->min->setText( ufkt->str_dmin );
	}
	else
	  editfunctionpage->customMinRange->setChecked(false);
	if (ufkt->usecustomxmax)
	{
	  editfunctionpage->customMaxRange->setChecked(true);
	  editfunctionpage->max->setText( ufkt->str_dmax );
	}
	else
		editfunctionpage->customMaxRange->setChecked(false);
	
	m_parameter =  ufkt->parameters;
	if( ufkt->use_slider == -1 )
		if ( ufkt->parameters.isEmpty() )
			editfunctionpage->useNoParameter->setChecked( true );
		else	
			editfunctionpage->useList->setChecked( true );
	else
	{
		editfunctionpage->useSlider->setChecked( true );
		editfunctionpage->listOfSliders->setCurrentItem( ufkt->use_slider );
	}
	
	editderivativespage->showDerivative1->setChecked( ufkt->f1_mode );
	editderivativespage->lineWidthDerivative1->setValue( ufkt->f1_linewidth );
	editderivativespage->colorDerivative1->setColor( ufkt->f1_color );
	editderivativespage->showDerivative2->setChecked( ufkt->f2_mode );
	editderivativespage->lineWidthDerivative2->setValue( ufkt->f2_linewidth );
	editderivativespage->colorDerivative2->setColor( ufkt->f2_color );
	editintegralpage->precision->setValue( ufkt->integral_precision );
	editintegralpage->lineWidth->setValue( ufkt->integral_linewidth );
	editintegralpage->color->setColor( ufkt->integral_color );
	
	if ( ufkt->integral_mode )
	{
		editintegralpage->showIntegral->setChecked( ufkt->integral_mode );
		editintegralpage->customPrecision->setChecked( ufkt->integral_use_precision );
		editintegralpage->txtInitX->setText(ufkt->str_startx);
		editintegralpage->txtInitY->setText(ufkt->str_starty);
		
	}

}

void EditFunction::accept()
{
  QString f_str( editfunctionpage->equation->text() );
        
        if ( m_id!=-1 )
                m_parser->fixFunctionName(f_str, XParser::Function, m_id);
        else
                m_parser->fixFunctionName(f_str, XParser::Function);
	if ( f_str.at(0)== 'x' || f_str.at(0)== 'y' || f_str.at(0)== 'r')
	{
		KMessageBox::error( this, i18n("You can only define plot functions in this dialog"));
		return;
	}
	
	Ufkt tmp_ufkt; //all settings are saved here until we know that no errors have appeared
	if( editfunctionpage->customMinRange->isChecked() )
	{
	  tmp_ufkt.usecustomxmin = true;
	  tmp_ufkt.str_dmin = editfunctionpage->min->text();
	  tmp_ufkt.dmin = m_parser->eval( editfunctionpage->min->text() );
	  if (m_parser->parserError() != 0)
	  {
	    showPage(0);
	    editfunctionpage->min->setFocus();
	    editfunctionpage->min->selectAll();
	    return;
	  }
	}
	else
	  tmp_ufkt.usecustomxmin = false;
	if( editfunctionpage->customMaxRange->isChecked() )
	{
	  tmp_ufkt.usecustomxmax = true;
	  tmp_ufkt.str_dmax= editfunctionpage->max->text();
	  tmp_ufkt.dmax = m_parser->eval( editfunctionpage->max->text() );
	  if (m_parser->parserError() != 0)
	  {
	    showPage(0);
	    editfunctionpage->max->setFocus();
	    editfunctionpage->max->selectAll();
	    return;
	  }
	}
	else
	  tmp_ufkt.usecustomxmax = false;
	
	if( tmp_ufkt.usecustomxmin && tmp_ufkt.usecustomxmax )
	{
		if ( tmp_ufkt.dmin >=  tmp_ufkt.dmax)
		{
			KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
			showPage(0);
			editfunctionpage->min->setFocus();
			editfunctionpage->min->selectAll();
			return;
		}
		
		if (  tmp_ufkt.dmin<View::xmin || tmp_ufkt.dmax>View::xmax )
		{
			KMessageBox::error(this,i18n("Please insert a minimum and maximum range between %1 and %2").arg(View::xmin).arg(View::xmax) );
			showPage(0);
			editfunctionpage->min->setFocus();
			editfunctionpage->min->selectAll();
			return;
		}
	}

	tmp_ufkt.linewidth = editfunctionpage->lineWidth->value();
	tmp_ufkt.color = editfunctionpage->color->color().rgb();
	
	if (editintegralpage->showIntegral->isChecked() )
	{
		double initx = m_parser->eval(editintegralpage->txtInitX->text());
		tmp_ufkt.startx = initx;
		tmp_ufkt.str_startx = editintegralpage->txtInitX->text();
		if (m_parser->parserError(false) != 0)
		{
			KMessageBox::error(this,i18n("Please insert a valid x-value"));
			showPage(2);
			editintegralpage->txtInitX->setFocus();
                        editintegralpage->txtInitX->selectAll();
			return;
		}
		
		double inity = m_parser->eval(editintegralpage->txtInitY->text());
		tmp_ufkt.starty = inity;
		tmp_ufkt.str_starty = editintegralpage->txtInitY->text();
		if (m_parser->parserError(false) != 0)
		{
			KMessageBox::error(this,i18n("Please insert a valid y-value"));
			showPage(2);
			editintegralpage->txtInitY->setFocus();
			editintegralpage->txtInitY->selectAll();
			return;
		}
		tmp_ufkt.integral_mode = 1;
	}
	else
		tmp_ufkt.integral_mode = 0;

	tmp_ufkt.integral_color = editintegralpage->color->color().rgb();
	tmp_ufkt.integral_use_precision = editintegralpage->customPrecision->isChecked();
	tmp_ufkt.integral_precision = editintegralpage->precision->value();
	tmp_ufkt.integral_linewidth = editintegralpage->lineWidth->value();

        tmp_ufkt.f_mode = !editfunctionpage->hide->isChecked();
	
	if( editfunctionpage->useSlider->isChecked() )
		tmp_ufkt.use_slider = editfunctionpage->listOfSliders->currentItem(); //specify which slider that will be used
	else
	{
		tmp_ufkt.use_slider = -1;
		if ( editfunctionpage->useNoParameter->isChecked() || m_parameter.isEmpty() )
			m_parameter.clear();
		else
		{
			tmp_ufkt.parameters = m_parameter;
		}
			
	}

	tmp_ufkt.f1_mode =  editderivativespage->showDerivative1->isChecked();
	tmp_ufkt.f1_linewidth = editderivativespage->lineWidthDerivative1->value();
	tmp_ufkt.f1_color = editderivativespage->colorDerivative1->color().rgb();
	
	tmp_ufkt.f2_mode =  editderivativespage->showDerivative2->isChecked();
	tmp_ufkt.f2_linewidth = editderivativespage->lineWidthDerivative2->value();
	tmp_ufkt.f2_color = editderivativespage->colorDerivative2->color().rgb();
	
	if ( f_str.contains('y') != 0 && ( tmp_ufkt.f_mode || tmp_ufkt.f1_mode || tmp_ufkt.f2_mode) )
	{
		KMessageBox::error( this, i18n( "Recursive function is only allowed when drawing integral graphs") );
		return;
        }
        
        Ufkt *added_ufkt;
        if( m_id != -1 )  //when editing a function: 
        {
                int const ix = m_parser->ixValue(m_id);
                if ( ix == -1) //The function could have been deleted
                {
                        KMessageBox::error(this,i18n("Function could not be found"));
                        return;
                }
                added_ufkt =  &m_parser->ufkt[ix];
                QString const old_fstr = added_ufkt->fstr;
                if((  (!m_parameter.isEmpty() && editfunctionpage->useList->isChecked() ) || editfunctionpage->useSlider->isChecked() ) && !functionHas2Arguments() )
                        fixFunctionArguments(f_str); //adding an extra argument for the parameter value
                added_ufkt->fstr = f_str;
                m_parser->reparse(added_ufkt); //reparse the funcion
                if ( m_parser->parserError() != 0)
                {
                        added_ufkt->fstr = old_fstr;
                        m_parser->reparse(added_ufkt); 
                        raise();
                        showPage(0);
                        editfunctionpage->equation->setFocus();
                        editfunctionpage->equation->selectAll();
                        return;
                }
        }
        else //creating a new function
        {
                if((  (!m_parameter.isEmpty() && editfunctionpage->useList->isChecked() ) || editfunctionpage->useSlider->isChecked() ) && !functionHas2Arguments() )
                        fixFunctionArguments(f_str); //adding an extra argument for the parameter value
                int const id = m_parser->addfkt( f_str ); //create a new function otherwise
                if( id == -1)
                {
                        m_parser->parserError();
                        raise();
                        showPage(0);
                        editfunctionpage->equation->setFocus();
                        editfunctionpage->equation->selectAll();
                        return;
                }
                added_ufkt =  &m_parser->ufkt.last();
        }
        //save all settings in the function now when we know no errors have appeared
	added_ufkt->f_mode = tmp_ufkt.f_mode;
	added_ufkt->f1_mode = tmp_ufkt.f1_mode;
	added_ufkt->f2_mode = tmp_ufkt.f2_mode;
	added_ufkt->integral_mode = tmp_ufkt.integral_mode;
	added_ufkt->integral_use_precision = tmp_ufkt.integral_use_precision;
	added_ufkt->linewidth = tmp_ufkt.linewidth;
	added_ufkt->f1_linewidth = tmp_ufkt.f1_linewidth;
	added_ufkt->f2_linewidth = tmp_ufkt.f2_linewidth;
	added_ufkt->integral_linewidth = tmp_ufkt.integral_linewidth;
	added_ufkt->str_dmin = tmp_ufkt.str_dmin;
	added_ufkt->str_dmax = tmp_ufkt.str_dmax;
	added_ufkt->dmin = tmp_ufkt.dmin;
	added_ufkt->dmax = tmp_ufkt.dmax;
	added_ufkt->str_startx = tmp_ufkt.str_startx;
	added_ufkt->str_starty = tmp_ufkt.str_starty;
	added_ufkt->oldx = tmp_ufkt.oldx;
	added_ufkt->starty = tmp_ufkt.starty;
	added_ufkt->startx = tmp_ufkt.startx;
	added_ufkt->integral_precision = tmp_ufkt.integral_precision;
	added_ufkt->color = tmp_ufkt.color;
	added_ufkt->f1_color = tmp_ufkt.f1_color;
	added_ufkt->f2_color = tmp_ufkt.f2_color;
	added_ufkt->integral_color = tmp_ufkt.integral_color;
	added_ufkt->parameters = tmp_ufkt.parameters;
	added_ufkt->use_slider = tmp_ufkt.use_slider;
	added_ufkt->usecustomxmin = tmp_ufkt.usecustomxmin;
	added_ufkt->usecustomxmax = tmp_ufkt.usecustomxmax;
        
    m_updatedfunction = added_ufkt;
	
	// call inherited method
	KDialogBase::accept();
}

Ufkt * EditFunction::functionItem()
{
    return m_updatedfunction;
}

void EditFunction::slotHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}

bool EditFunction::functionHas2Arguments()
{
	int const openBracket = editfunctionpage->equation->text().find( "(" );
	int const closeBracket = editfunctionpage->equation->text().find( ")" );
	return editfunctionpage->equation->text().mid( openBracket+1, closeBracket-openBracket-1 ).find( "," ) != -1;
}
void EditFunction::cmdParameter_clicked()
{
	editfunctionpage->useList->setChecked(true);
	KParameterEditor *dlg = new KParameterEditor(m_parser,&m_parameter);
	dlg->show();
}
void EditFunction::noParameter_toggled(bool status)
{
	if (status)
	{
		editfunctionpage->cmdParameter->setEnabled(false);
		editfunctionpage->listOfSliders->setEnabled(false);
	}
}

void EditFunction::customMinRange_toggled(bool status)
{
  if (status)
    editfunctionpage->min->setEnabled(true);
  else
    editfunctionpage->min->setEnabled(false);
}

void EditFunction::customMaxRange_toggled(bool status)
{
  if (status)
    editfunctionpage->max->setEnabled(true);
  else
    editfunctionpage->max->setEnabled(false);
}

void EditFunction::fixFunctionArguments(QString &f_str)
{
	int const openBracket = f_str.find( "(" );
	int const closeBracket = f_str.find( ")" );
	char parameter_name;
	if ( closeBracket-openBracket == 2) //the function atribute is only one character
	{
		char const function_name = f_str.at(openBracket+1).latin1();
		parameter_name = 'a';
		while ( parameter_name == function_name)
			parameter_name++;
	}
	else
		parameter_name = 'a';
	f_str.insert(closeBracket,parameter_name);
	f_str.insert(closeBracket,',');
}

