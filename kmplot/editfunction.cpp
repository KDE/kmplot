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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
#include "editantiderivativepage.h"
#include "kparametereditor.h"

EditFunction::EditFunction( XParser* parser, QWidget* parent, const char* name ) : 
	KDialogBase( IconList, "Caption", Help|Ok|Cancel, Ok, parent, name ), m_parser(parser)
{
	QVBox *page0 = addVBoxPage( i18n("Function"), i18n( "Function" ), SmallIcon( "func", 32 ) );
	editfunctionpage = new EditFunctionPage( page0 );
	QVBox *page1 = addVBoxPage( i18n("Derivatives"), i18n( "Derivatives" ), SmallIcon( "deriv_func", 32 ) );
	editderivativespage = new EditDerivativesPage( page1 );
	QVBox *page2 = addVBoxPage( i18n("Antiderivative"), i18n( "Antiderivative" ), SmallIcon( "anti_func", 32 ) );
	editantiderivativepage = new EditAntiderivativePage( page2 );
	for( int number = 0; number < SLIDER_COUNT; number++ )
	{
		editfunctionpage->listOfSliders->insertItem( QString( "Slider no. %1" ).arg( number ) );
	}
	connect( editfunctionpage->cmdParameter, SIGNAL ( clicked() ), this, SLOT( cmdParameter_clicked() ) );
}

void EditFunction::initDialog( int index )
{
	m_index = index;
	if( m_index == -1 ) clearWidgets();
	else setWidgets();
	editfunctionpage->equation->setFocus();
}

void EditFunction::clearWidgets()
{
	
	// Clear the Function page
	editfunctionpage->equation->clear();
	editfunctionpage->hide->setChecked( false );
	editfunctionpage->customRange->setChecked( false );
	editfunctionpage->min->clear();
	editfunctionpage->max->clear();
	editfunctionpage->lineWidth->setValue( m_parser->linewidth0 );
	editfunctionpage->color->setColor( m_parser->fktext[ m_parser->getNextIndex() ].color0 );
	
	// Clear the Derivatives page
	editderivativespage->showDerivative1->setChecked( false );
	editderivativespage->lineWidthDerivative1->setValue( editfunctionpage->lineWidth->value() );
	editderivativespage->colorDerivative1->setColor( editfunctionpage->color->color() );
	editderivativespage->showDerivative2->setChecked( false );
	editderivativespage->lineWidthDerivative2->setValue( editfunctionpage->lineWidth->value() );
	editderivativespage->colorDerivative2->setColor( editfunctionpage->color->color() );
	
	// Clear the Antiderivative page
	editantiderivativepage->precision->setValue( Settings::relativeStepWidth());
	editantiderivativepage->colorAntiderivative->setColor( editfunctionpage->color->color() );
	editantiderivativepage->lineWidthAntiderivative->setValue(editfunctionpage->lineWidth->value());
	
}

void EditFunction::setWidgets()
{
	m_parameter =  m_parser->fktext[ m_index ].str_parameter;
	editfunctionpage->equation->setText( m_parser->fktext[ m_index ].extstr );
	editfunctionpage->hide->setChecked( m_parser->fktext[ m_index ].f_mode == 0 );
	QStringList listOfParameters;
	for( int k_index = 0; k_index < m_parser->fktext[ m_index ].k_anz; k_index++ )
	{
		listOfParameters += QString::number( m_parser->fktext[ m_index ].k_liste[ k_index ] );
	}
	//editfunctionpage->parameters->setText( listOfParameters.join( "," ) );
	if (  m_parser->fktext[ m_index ].dmin != m_parser->fktext[ m_index ].dmax )
	{
		editfunctionpage->customRange->setChecked(true);
		editfunctionpage->min->setText( m_parser->fktext[ m_index ].str_dmin );
		editfunctionpage->max->setText( m_parser->fktext[ m_index ].str_dmax );
	}
	else
		editfunctionpage->customRange->setChecked(false);

	editfunctionpage->lineWidth->setValue( m_parser->fktext[ m_index ].linewidth );
	editfunctionpage->color->setColor( m_parser->fktext[ m_index ].color );
	if( m_parser->fktext[ m_index ].use_slider == -1 )
		editfunctionpage->useList->setChecked( true );
	else
	{
		editfunctionpage->useSlider->setChecked( true );
		editfunctionpage->listOfSliders->setCurrentItem( m_parser->fktext[ m_index ].use_slider );
	}
	
	editderivativespage->showDerivative1->setChecked( m_parser->fktext[ m_index ].f1_mode );
	editderivativespage->lineWidthDerivative1->setValue( m_parser->fktext[ m_index ].f1_linewidth );
	editderivativespage->colorDerivative1->setColor( m_parser->fktext[ m_index ].f1_color );
	editderivativespage->showDerivative2->setChecked( m_parser->fktext[ m_index ].f2_mode );
	editderivativespage->lineWidthDerivative2->setValue( m_parser->fktext[ m_index ].f2_linewidth );
	editderivativespage->colorDerivative2->setColor( m_parser->fktext[ m_index ].f2_color );
	editantiderivativepage->precision->setValue( m_parser->fktext[ m_index ].anti_precision );
	editantiderivativepage->lineWidthAntiderivative->setValue( m_parser->fktext[ m_index ].anti_linewidth );
	editantiderivativepage->colorAntiderivative->setColor( m_parser->fktext[ m_index ].anti_color );
	
	if ( m_parser->fktext[ m_index ].anti_mode )
	{
		editantiderivativepage->showAntiderivative->setChecked( m_parser->fktext[ m_index ].anti_mode );
		editantiderivativepage->customPrecision->setChecked( m_parser->fktext[ m_index ].anti_use_precision );
		editantiderivativepage->txtInitX->setText(m_parser->fktext[ m_index ].str_startx);
		editantiderivativepage->txtInitY->setText(m_parser->fktext[ m_index ].str_starty);
		
	}

}

void EditFunction::accept()
{
	if( !m_parameter.isEmpty() != 0 )
	{
		if( !functionHas2Arguments() && KMessageBox::warningYesNo( this, i18n( "You entered parameter values, but the function has no 2nd argument. The Definition should look like f(x,k)=k*x^2, for instance.\nDo you want to continue anyway?" ), i18n( "Missing 2nd Argument" ) ) != KMessageBox::Yes ) return;
	}
	else if( functionHas2Arguments() && KMessageBox::warningYesNo( this, i18n( "Function has 2 arguments, but you did not specify any parameter values.\nDo you want to continue anyway?" ), i18n( "Missing Parameter Values" ) ) != KMessageBox::Yes )
		return;
	
	QString f_str(functionItem() );
	m_parser->fixFunctionName(f_str);
	int index;
	if( m_index != -1 )  //when editing a function: 
	{
		index = m_index; //use the right function-index
		QString old_fstr = m_parser->ufkt[index].fstr;
		m_parser->ufkt[index].fstr = f_str;
		m_parser->reparse(index); //reparse the funcion
		if ( m_parser->errmsg() != 0)
		{
			m_parser->ufkt[index].fstr = old_fstr;
			m_parser->reparse(index); 
			this->raise();
			showPage(0);
			editfunctionpage->equation->setFocus();
			editfunctionpage->equation->selectAll();
			return;
		}
	}
	else
		index = m_parser->addfkt( f_str ); //create a new function otherwise
	
	if( index == -1) 
	{
		m_parser->errmsg();
		this->raise();
		showPage(0);
		editfunctionpage->equation->setFocus();
		editfunctionpage->equation->selectAll();
		return;
	}
	
	XParser::FktExt tmp_fktext; //all settings are saved here until we know that no errors have appeared
	tmp_fktext.extstr = f_str;
	
	if( editfunctionpage->customRange->isChecked() )
	{
		tmp_fktext.str_dmin = editfunctionpage->min->text();
		tmp_fktext.dmin = m_parser->eval( editfunctionpage->min->text() );
		if (m_parser->errmsg() != 0)
		{
			showPage(0);
			editfunctionpage->min->setFocus();
			editfunctionpage->min->selectAll();
			if( m_index == -1 ) m_parser->delfkt(index);
			return;
		}
		tmp_fktext.str_dmax= editfunctionpage->max->text();
		tmp_fktext.dmax = m_parser->eval( editfunctionpage->max->text() );
		if (m_parser->errmsg() != 0)
		{
			showPage(0);
			editfunctionpage->max->setFocus();
			editfunctionpage->max->selectAll();
			if( m_index == -1 ) m_parser->delfkt(index);
			return;
		}
		
		if ( tmp_fktext.dmin >=  tmp_fktext.dmax)
		{
			KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
			showPage(0);
			editfunctionpage->min->setFocus();
			editfunctionpage->min->selectAll();
			if( m_index == -1 ) m_parser->delfkt(index);
			return;
		}
		
		if (  tmp_fktext.dmin<View::xmin || tmp_fktext.dmax>View::xmax )
		{
			KMessageBox::error(this,i18n("Please insert a minimum and maximum range between %1 and %2").arg(View::xmin).arg(View::xmax) );
			showPage(0);
			editfunctionpage->min->setFocus();
			editfunctionpage->min->selectAll();
			if( m_index == -1 ) m_parser->delfkt(index);
			return;
		}
	}
	else
	{
		tmp_fktext.str_dmin ="0";
		tmp_fktext.dmin = 0;
		tmp_fktext.str_dmax = "0";
		tmp_fktext.dmax = 0;
	}
	
	if( editfunctionpage->useList->isChecked() )
		tmp_fktext.use_slider = -1;
	else
		tmp_fktext.use_slider = editfunctionpage->listOfSliders->currentItem();

	tmp_fktext.linewidth = editfunctionpage->lineWidth->value();
	tmp_fktext.color = editfunctionpage->color->color().rgb();
	
	if (editantiderivativepage->showAntiderivative->isChecked() )
	{
		double initx = m_parser->eval(editantiderivativepage->txtInitX->text());
		tmp_fktext.startx = initx;
		tmp_fktext.str_startx = editantiderivativepage->txtInitX->text();
		if (m_parser->err != 0)
		{
			KMessageBox::error(this,i18n("Please insert a valid x-value"));
			showPage(2);
			editantiderivativepage->txtInitX->setFocus();
			editantiderivativepage->txtInitX->selectAll();
			if( m_index == -1 ) m_parser->delfkt(index);
			return;
		}
		
		double inity = m_parser->eval(editantiderivativepage->txtInitY->text());
		tmp_fktext.starty = inity;
		tmp_fktext.str_starty = editantiderivativepage->txtInitY->text();
		if (m_parser->err != 0)
		{
			KMessageBox::error(this,i18n("Please insert a valid y-value"));
			showPage(2);
			editantiderivativepage->txtInitY->setFocus();
			editantiderivativepage->txtInitY->selectAll();
			if( m_index == -1 ) m_parser->delfkt(index);
			return;
		}
		if ( tmp_fktext.dmin!=tmp_fktext.dmax && ( initx<tmp_fktext.dmin || initx>tmp_fktext.dmax) )
		{
			KMessageBox::error(this,i18n("Please insert an initial x-value in the range between %1 and %2").arg(tmp_fktext.dmin).arg( tmp_fktext.dmax) );
			showPage(2);
			editantiderivativepage->txtInitX->setFocus();
			editantiderivativepage->txtInitX->selectAll();
			if( m_index == -1 ) m_parser->delfkt(index);
			return;
		}
		if ( tmp_fktext.dmin!=tmp_fktext.dmax && ( initx<View::xmin || initx>View::xmax) )
		{
			KMessageBox::error(this,i18n("Please insert an initial x-value in the range between %1 and %2").arg(View::xmax).arg( View::xmax) );
			showPage(2);
			editantiderivativepage->txtInitX->setFocus();
			editantiderivativepage->txtInitX->selectAll();
			if( m_index == -1 ) m_parser->delfkt(index);
			return;
		}
		
		tmp_fktext.anti_mode = 1;
	}
	else
		tmp_fktext.anti_mode = 0;

	tmp_fktext.anti_color = editantiderivativepage->colorAntiderivative->color().rgb();
	tmp_fktext.anti_use_precision = editantiderivativepage->customPrecision->isChecked();
	tmp_fktext.anti_precision = editantiderivativepage->precision->value();
	tmp_fktext.anti_linewidth = editantiderivativepage->lineWidthAntiderivative->value();
	
	if( editfunctionpage->hide->isChecked() )
		tmp_fktext.f_mode = 0;
	else
		tmp_fktext.f_mode = 1;
	
	tmp_fktext.k_anz = 0;
	if( !m_parameter.isEmpty() )
	{
		tmp_fktext.str_parameter = m_parameter;
		for( QStringList::Iterator it = m_parameter.begin(); it != m_parameter.end(); ++it )
		{
			tmp_fktext.k_liste[ tmp_fktext.k_anz ] = m_parser->eval(( *it ) );
			tmp_fktext.k_anz++;
		}
	}
	
	if( editderivativespage->showDerivative1->isChecked() )
		tmp_fktext.f1_mode = 1;
	else
		tmp_fktext.f1_mode = 0;
	
	tmp_fktext.f1_linewidth = editderivativespage->lineWidthDerivative1->value();
	tmp_fktext.f1_color = editderivativespage->colorDerivative1->color().rgb();
	
	if( editderivativespage->showDerivative2->isChecked() )
		tmp_fktext.f2_mode = 1;
	else
		tmp_fktext.f2_mode = 0;

	tmp_fktext.f2_linewidth = editderivativespage->lineWidthDerivative2->value();
	tmp_fktext.f2_color = editderivativespage->colorDerivative2->color().rgb();
	
	if ( f_str.contains('y') != 0 && ( tmp_fktext.f_mode || tmp_fktext.f1_mode || tmp_fktext.f2_mode) )
	{
		KMessageBox::error( this, i18n( "Recursive function is only allowed when drawing anti-derivatives graphs") );
		if( m_index == -1 ) m_parser->delfkt(index);
		return;
	}
	
	m_parser->fktext[index] = tmp_fktext;
	
	// call inherited method
	KDialogBase::accept();
}

const QString EditFunction::functionItem()
{
	return editfunctionpage->equation->text();
}

void EditFunction::slotHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}

bool EditFunction::functionHas2Arguments()
{
	int openBracket = editfunctionpage->equation->text().find( "(" );
	int closeBracket = editfunctionpage->equation->text().find( ")" );
	return editfunctionpage->equation->text().mid( openBracket+1, closeBracket-openBracket-1 ).find( "," ) != -1;
}
void EditFunction::cmdParameter_clicked()
{
	KParameterEditor *dlg = new KParameterEditor(m_parser,&m_parameter);
	dlg->show();
}