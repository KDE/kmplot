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
#include <qframe.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qstringlist.h>

#include <qdialog.h>
#include <qlabel.h>

// KDE includes
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>

#include <kdebug.h>

// local includes
#include "editfunction.h"
#include "editfunction.moc"
#include "xparser.h"
#include "editfunctionpage.h"
#include "editderivativespage.h"
#include "editantiderivativepage.h"
#include "kparametereditor.h"

EditFunction::EditFunction( XParser* parser, QWidget* parent, const char* name ) : 
	KDialogBase( IconList, "Caption", Help|Ok|Cancel, Ok, parent, name )
{
	QVBox *page0 = addVBoxPage( i18n("Function"), i18n( "Function" ), SmallIcon( "func", 32 ) );
	editfunctionpage = new EditFunctionPage( page0 );
	QVBox *page1 = addVBoxPage( i18n("Derivatives"), i18n( "Derivatives" ), SmallIcon( "deriv_func", 32 ) );
	editderivativespage = new EditDerivativesPage( page1 );
	QVBox *page2 = addVBoxPage( i18n("Antiderivative"), i18n( "Antiderivative" ), SmallIcon( "anti_func", 32 ) );
	editantiderivativepage = new EditAntiderivativePage( page2 );
	m_parser = parser;
	connect( editfunctionpage->cmdParameter, SIGNAL (clicked() ), this, SLOT(cmdParameter_clicked() ) );
	connect( editfunctionpage->hasParameters, SIGNAL (clicked() ), this, SLOT(hasParameters_clicked() ) );
}

void EditFunction::initDialog( int index )
{
	m_index = index;
	if( m_index == -1 ) clearWidgets();
	else setWidgets();
}

void EditFunction::clearWidgets()
{
	
	// Clear the Function page
	editfunctionpage->equation->clear();
	editfunctionpage->hide->setChecked( false );
	editfunctionpage->hasParameters->setChecked( false );
	editfunctionpage->customRange->setChecked( false );
	editfunctionpage->min->clear();
	editfunctionpage->max->clear();
	editfunctionpage->lineWidth->setValue( m_parser->dicke0 );
	editfunctionpage->color->setColor( m_parser->fktext[ m_parser->getNextIndex() ].color );
	
	// Clear the Derivatives page
	editderivativespage->showDerivative1->setChecked( false );
	editderivativespage->lineWidthDerivative1->setValue( 1 );
	editderivativespage->colorDerivative1->setColor( editfunctionpage->color->color() );
	editderivativespage->showDerivative2->setChecked( false );
	editderivativespage->lineWidthDerivative2->setValue( 1 );
	editderivativespage->colorDerivative2->setColor( editfunctionpage->color->color() );
	
	// Clear the Antiderivative page
	editantiderivativepage->precision->setValue( Settings::relativeStepWidth());
	editantiderivativepage->colorAntiderivative->setColor( editfunctionpage->color->color() );
	
}

void EditFunction::setWidgets()
{
	m_parameter =  m_parser->fktext[ m_index ].str_parameter;
	editfunctionpage->equation->setText( m_parser->fktext[ m_index ].extstr );
	editfunctionpage->hide->setChecked( m_parser->fktext[ m_index ].f_mode == 0 );
	editfunctionpage->hasParameters->setChecked( m_parser->fktext[ m_index ].k_anz != 0 );
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
	
	editderivativespage->showDerivative1->setChecked( m_parser->fktext[ m_index ].f1_mode );
	editderivativespage->lineWidthDerivative1->setValue( m_parser->fktext[ m_index ].f1_linewidth );
	editderivativespage->colorDerivative1->setColor( m_parser->fktext[ m_index ].f1_color );
	editderivativespage->showDerivative2->setChecked( m_parser->fktext[ m_index ].f2_mode );
	editderivativespage->lineWidthDerivative2->setValue( m_parser->fktext[ m_index ].f2_linewidth );
	editderivativespage->colorDerivative2->setColor( m_parser->fktext[ m_index ].f2_color );
	
	editantiderivativepage->customPrecision->setChecked( m_parser->fktext[ m_index ].anti_use_precision );
	editantiderivativepage->precision->setValue( m_parser->fktext[ m_index ].anti_precision );
	editantiderivativepage->lineWidthAntiderivative->setValue( m_parser->fktext[ m_index ].anti_linewidth );
	editantiderivativepage->colorAntiderivative->setColor( m_parser->fktext[ m_index ].anti_color );
	
	if ( m_parser->fktext[ m_index ].anti_mode )
	{
		editantiderivativepage->showAntiderivative->setChecked( m_parser->fktext[ m_index ].anti_mode );
		editantiderivativepage->txtInitX->setText(m_parser->fktext[ m_index ].str_startx);
		editantiderivativepage->txtInitY->setText(m_parser->fktext[ m_index ].str_starty);
		
	}

}

void EditFunction::accept()
{
	if( editfunctionpage->hasParameters->isChecked() )
	{
		if( !functionHas2Arguments() && KMessageBox::warningYesNo( this, i18n( "You entered parameter values, but the function has no 2nd argument. The Definition should look like f(x,k)=k*x^2, for instance.\nDo you want to continue anyway?" ), i18n( "Missing 2nd Argument" ) ) != KMessageBox::Yes ) return;
	}
	else if( functionHas2Arguments() && KMessageBox::warningYesNo( this, i18n( "Function has 2 arguments, but you did not specify any parameter values.\nDo you want to continue anyway?" ), i18n( "Missing Parameter Values" ) ) != KMessageBox::Yes )
		return;
	
	// if we are editing an existing function, first delete the old one
	if( m_index != -1 ) 
	{
		m_parser->delfkt( m_index );
		m_index = -1;
	}
	
	
	QString f_str(functionItem() );
	m_parser->fixFunctionName(f_str);
	
	int index = m_parser->addfkt( f_str );
	if( index == -1 ) 
	{
		m_parser->errmsg();
		this->raise();
		showPage(0);
		editfunctionpage->equation->setFocus();
		editfunctionpage->equation->selectAll();
		return;
	}
	m_parser->fktext[ index ].extstr = f_str;
	m_parser->getext( index );
	
	if( editfunctionpage->customRange->isChecked() )
	{
		// TODO: check empty boundaries
		
		m_parser->fktext[ index ].str_dmin = editfunctionpage->min->text();
		m_parser->fktext[ index ].dmin = m_parser->eval( editfunctionpage->min->text() );
		if (m_parser->errmsg() != 0)
		{
			showPage(0);
			editfunctionpage->min->setFocus();
			editfunctionpage->min->selectAll();
			m_parser->delfkt( index );
			return;
		}
		m_parser->fktext[ index ].str_dmax = editfunctionpage->max->text();
		m_parser->fktext[ index ].dmax = m_parser->eval( editfunctionpage->max->text() );
		if (m_parser->errmsg() != 0)
		{
			showPage(0);
			editfunctionpage->max->setFocus();
			editfunctionpage->max->selectAll();
			m_parser->delfkt( index );
			return;
		}
		if ( m_parser->fktext[ index ].dmin >=  m_parser->fktext[ index ].dmax)
		{
			KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
			showPage(0);
			editfunctionpage->min->setFocus();
			editfunctionpage->min->selectAll();
			m_parser->delfkt( index );
			return;
		}
	}
	else
	{
		m_parser->fktext[ index ].str_dmin ="0";
		m_parser->fktext[ index ].dmin = 0;
		m_parser->fktext[ index ].str_dmax = "0";
		m_parser->fktext[ index ].dmax = 0;
	}
	
	if (editantiderivativepage->showAntiderivative->isChecked() )
	{
		double initx = m_parser->eval(editantiderivativepage->txtInitX->text());
		m_parser->fktext[index].startx = initx;
		m_parser->fktext[index].str_startx = editantiderivativepage->txtInitX->text();
		if (m_parser->err != 0)
		{
			KMessageBox::error(this,i18n("Please insert a valid x-value"));
			showPage(2);
			editantiderivativepage->txtInitX->setFocus();
			editantiderivativepage->txtInitX->selectAll();
			m_parser->delfkt( index );
			return;
		}
		
		double inity = m_parser->eval(editantiderivativepage->txtInitY->text());
		m_parser->fktext[index].starty = inity;
		m_parser->fktext[index].str_starty = editantiderivativepage->txtInitY->text();
		if (m_parser->err != 0)
		{
			KMessageBox::error(this,i18n("Please insert a valid y-value"));
			showPage(2);
			editantiderivativepage->txtInitY->setFocus();
			editantiderivativepage->txtInitY->selectAll();
			m_parser->delfkt( index );
			return;
		}
		if ( m_parser->fktext[ index ].dmin!=m_parser->fktext[ index ].dmax && ( initx<m_parser->fktext[ index ].dmin || initx>m_parser->fktext[ index ].dmax) )
		{
			KMessageBox::error(this,i18n("Please insert a initial x-value in the range between %1 and %2").arg(m_parser->fktext[ index ].str_dmin, m_parser->fktext[ index ].str_dmax) );
			showPage(2);
			editantiderivativepage->txtInitX->setFocus();
			editantiderivativepage->txtInitX->selectAll();
			m_parser->delfkt( index );
			return;
		}

		m_parser->fktext[index].anti_mode = 1;
		m_parser->fktext[ index ].anti_color = editantiderivativepage->colorAntiderivative->color().rgb();
		m_parser->fktext[index].anti_use_precision = editantiderivativepage->customPrecision->isChecked();
		m_parser->fktext[index].anti_precision = editantiderivativepage->precision->value();
		m_parser->fktext[index].anti_linewidth = editantiderivativepage->lineWidthAntiderivative->value();
	}
	else
		m_parser->fktext[index].anti_mode = 0;

	if( editfunctionpage->hide->isChecked() )
		m_parser->fktext[ index ].f_mode = 0;
		
	if( editfunctionpage->hasParameters->isChecked() )
	{
		m_parser->fktext[ index ].str_parameter = m_parameter;
		m_parser->fktext[ index ].k_anz = 0;
		for( QStringList::Iterator it = m_parameter.begin(); it != m_parameter.end(); ++it )
		{
			m_parser->fktext[ index ].k_liste[ m_parser->fktext[ index ].k_anz ] = m_parser->eval(( *it ) );
			m_parser->fktext[ index ].k_anz++;
		}
	}
	
	m_parser->fktext[ index ].linewidth = editfunctionpage->lineWidth->value();
	m_parser->fktext[ index ].color = editfunctionpage->color->color().rgb();
	
	editfunctionpage->equation->setText(f_str);
	
	if( editderivativespage->showDerivative1->isChecked() )
	{
		m_parser->fktext[ index ].f1_mode = 1;
		m_parser->fktext[ index ].f1_linewidth = editderivativespage->lineWidthDerivative1->value();
		m_parser->fktext[ index ].f1_color = editderivativespage->colorDerivative1->color().rgb();
	}
	else
		m_parser->fktext[ index ].f1_mode = 0;
	
	if( editderivativespage->showDerivative2->isChecked() )
	{
		m_parser->fktext[ index ].f2_mode = 1;
		m_parser->fktext[ index ].f2_linewidth = editderivativespage->lineWidthDerivative2->value();
		m_parser->fktext[ index ].f2_color = editderivativespage->colorDerivative2->color().rgb();
	}
	else
		m_parser->fktext[ index ].f2_mode = 0;
	
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
	KParameterEditor *dlg = new KParameterEditor(m_parser, &m_parameter);
	dlg->show();
}

void EditFunction::hasParameters_clicked()
{
	if ( editfunctionpage->hasParameters->isChecked() )
		editfunctionpage->cmdParameter->setEnabled(true);
	else
		editfunctionpage->cmdParameter->setEnabled(false);
}