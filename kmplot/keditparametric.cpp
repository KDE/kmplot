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

// KDE includes
#include <kapplication.h>
#include <kcolorbutton.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include <kdebug.h>

// local includes
#include "keditparametric.h"
#include "keditparametric.moc"
#include "xparser.h"
#include "View.h"

KEditParametric::KEditParametric( XParser* parser, QWidget* parent, const char* name ) : 
	QEditParametric( parent, name ), m_parser(parser)
{
}

void KEditParametric::initDialog( int x_id, int y_id)
{
	m_x_id = x_id;
	m_y_id = y_id;
	if( m_x_id == -1 ) clearWidgets();
	else setWidgets();
}

void KEditParametric::clearWidgets()
{
	kLineEditName->clear();
	kLineEditXFunction->clear();
	kLineEditYFunction->clear();
	checkBoxHide->setChecked( false );
	checkBoxRange->setChecked( false );
	min->clear();
	max->clear();
	kIntNumInputLineWidth->setValue( m_parser->linewidth0 );
	kColorButtonColor->setColor( m_parser->defaultColor(m_parser->getNextIndex() ) );
}

void KEditParametric::setWidgets()
{
	QString name, expression;
	splitEquation( m_parser->fktext[ m_x_id ].extstr, name, expression );
	kLineEditName->setText( name );
	kLineEditXFunction->setText( expression );
	splitEquation( m_parser->fktext[ m_y_id ].extstr, name, expression );
	kLineEditYFunction->setText( expression );
	checkBoxHide->setChecked( m_parser->fktext[ m_x_id ].f_mode == 0 );
	if (  m_parser->fktext[ m_x_id ].dmin != m_parser->fktext[ m_x_id ].dmax )
	{
		checkBoxRange->setChecked(true);
		min->setText( m_parser->fktext[ m_x_id ].str_dmin );
		max->setText( m_parser->fktext[ m_x_id ].str_dmax );
	}
	else
		checkBoxRange->setChecked(false);
	kIntNumInputLineWidth->setValue( m_parser->fktext[ m_x_id ].linewidth );
	kColorButtonColor->setColor( m_parser->fktext[ m_x_id ].color );
}

void KEditParametric::accept()
{
	if  ( kLineEditXFunction->text().contains('y') != 0 ||  kLineEditYFunction->text().contains('y') != 0)
	{
		KMessageBox::error( this, i18n( "Recursive function not allowed"));
		kLineEditXFunction->setFocus();
		kLineEditXFunction->selectAll();
		return;
	}
	
	// find a name not already used 
	if( kLineEditName->text().isEmpty() )
		kLineEditName->setText( newName() );

	int id;
        Parser::Ufkt *tmp_ufkt;
	if( m_x_id != -1 )  //when editing a function: 
	{
                int const ix = m_parser->ixValue(m_x_id);
                if ( ix == -1) //The function could have been deleted
                {
                        KMessageBox::error(this,i18n("Function could not be found"));
                        return;
                }
		id = m_x_id; //use the right function-id
                tmp_ufkt = &m_parser->ufkt[ix];
		QString old_fstr = tmp_ufkt->fstr;
		tmp_ufkt->fstr = xFunction();
		m_parser->reparse(tmp_ufkt); //reparse the funcion
		if ( m_parser->errmsg() != 0)
		{
			tmp_ufkt->fstr = old_fstr;
			m_parser->reparse(tmp_ufkt); 
			this->raise();
			kLineEditXFunction->setFocus();
			kLineEditXFunction->selectAll();
			return;
		}
	}
	else
        {
		id = m_parser->addfkt(xFunction() );
                if( id == -1 ) 
                {
                        m_parser->errmsg();
                        this->raise();
                        kLineEditXFunction->setFocus();
                        kLineEditXFunction->selectAll();
                        return;
                }
                tmp_ufkt =  &m_parser->ufkt.last();
        }

	XParser::FktExt fktext_x;
	fktext_x.extstr = xFunction();
	
	if( checkBoxHide->isChecked() )
		fktext_x.f_mode = 0;
	else
		fktext_x.f_mode = 1;
	
	if( checkBoxRange->isChecked() )
	{
		fktext_x.str_dmin = min->text();
		fktext_x.dmin = m_parser->eval( min->text() );
		if ( m_parser->errmsg())
		{
			min->setFocus();
			min->selectAll();
                        if( m_x_id == -1 ) m_parser->Parser::delfkt( tmp_ufkt );
			return;
		}
		fktext_x.str_dmax = max->text();
		fktext_x.dmax = m_parser->eval( max->text() );
		if ( m_parser->errmsg())
		{
			max->setFocus();
			max->selectAll();
                        if( m_x_id == -1 ) m_parser->Parser::delfkt( tmp_ufkt );
			return;
		}
		if ( fktext_x.dmin >=  fktext_x.dmax)
		{
			KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
			min->setFocus();
			min->selectAll();
                        if( m_x_id == -1 ) m_parser->Parser::delfkt( tmp_ufkt );
			return;
		}
	}
	else
	{
		fktext_x.str_dmin ="0";
		fktext_x.dmin = 0;
		fktext_x.str_dmax = "0";
		fktext_x.dmax = 0;
	}
	
	fktext_x.linewidth = kIntNumInputLineWidth->value();
	fktext_x.color = kColorButtonColor->color().rgb();
	fktext_x.f1_color = fktext_x.f2_color = fktext_x.integral_color = fktext_x.color;
	fktext_x.integral_mode = 0;
	fktext_x.f1_mode = fktext_x.f1_mode;
	fktext_x.f2_mode = 0;
	fktext_x.use_slider = -1;
        
        //save all settings in the function now when we now no errors have appeared
        if( m_x_id == -1 )
        {
                fktext_x.id = tmp_ufkt->id;
                m_parser->fktext.append(fktext_x);
        }
        else
        {
                fktext_x.id = id;
                m_parser->fktext[id] = fktext_x;
        }
	
        XParser::FktExt fktext_y;
        tmp_ufkt = 0;
        fktext_y = fktext_x;
        fktext_y.extstr = yFunction();  
                
	if( m_y_id != -1 )  //when editing a function: 
	{
                tmp_ufkt = &m_parser->ufkt[m_parser->ixValue(m_y_id)];
		id = m_y_id; //use the right function-id
		QString old_fstr = tmp_ufkt->fstr;
		tmp_ufkt->fstr = yFunction();
		m_parser->reparse(tmp_ufkt); //reparse the funcion
		if ( m_parser->errmsg() != 0) //when something went wrong:
		{
			tmp_ufkt->fstr = old_fstr; //go back to the old expression
			m_parser->reparse(tmp_ufkt);  //reparse
			this->raise();
			kLineEditXFunction->setFocus();
			kLineEditXFunction->selectAll();
			return;
		}
	}
	else
        {
		id = m_parser->addfkt(yFunction() );
                tmp_ufkt =  &m_parser->ufkt.last();
        }

	if( id == -1 ) 
	{
		m_parser->errmsg();
		this->raise();
		kLineEditYFunction->setFocus();
		kLineEditYFunction->selectAll();
		return;
	}
	fktext_y.extstr = yFunction();
        
         //save all settings in the function now when we now no errors have appeared
        if( m_y_id == -1 )
        {
                fktext_y.id = tmp_ufkt->id;
                m_parser->fktext.append(fktext_y);
        }
        else
        {
                fktext_y.id = id;
                m_parser->fktext[id] = fktext_y;
        }
	
	// call inherited method
	QEditParametric::accept(); //update the function name in FktDlg
}


QString KEditParametric::newName()
{
	int i = 0;
	QString name;
	// prepend the correct prefix
	name = "xf%1";
	/*do
	{
		i++;
	}
        while( m_parser->getfix( name.arg( i ) ) != -1 );
	*/
	// cut off prefix again, will be added later
	name = name.right( name.length()-1 );
	return name.arg( i );
}

QString KEditParametric::xFunction()
{
	return "x" + kLineEditName->text() + "(t)=" + kLineEditXFunction->text();
}

void KEditParametric::splitEquation( const QString equation, QString &name, QString &expression )
{
	int start = 0;
	if( equation[ 0 ] == 'r' || equation[ 0 ] == 'x' || equation[ 0 ] == 'y' ) start++;
	int length = equation.find( '(' ) - start;
	name = equation.mid( start, length );
	
	expression = equation.section( '=', 1, 1 );
}

QString KEditParametric::yFunction()
{
	return "y" + kLineEditName->text() + "(t)=" + kLineEditYFunction->text();
}

const QString KEditParametric::functionItem()
{
	return xFunction()+";"+yFunction();
}

void KEditParametric::slotHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}
