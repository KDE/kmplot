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
#include "keditpolar.h"
#include "keditpolar.moc"
#include "xparser.h"
#include "View.h"

KEditPolar::KEditPolar( XParser* parser, QWidget* parent, const char* name ) : 
	QEditPolar( parent, name ),m_parser(parser)
{
}

void KEditPolar::initDialog( int id )
{
	m_id = id;
	if( m_id == -1 ) clearWidgets();
	else setWidgets();
}

void KEditPolar::clearWidgets()
{
	kLineEditYFunction->clear();
	checkBoxHide->setChecked( false );
	checkBoxRange->setChecked( false );
	min->clear();
	max->clear();
	kIntNumInputLineWidth->setValue( m_parser->linewidth0 );
	kColorButtonColor->setColor( m_parser->defaultColor(m_parser->getNextIndex() ) );
}

void KEditPolar::setWidgets()
{
	QString function = m_parser->fktext[ m_id ].extstr;
	function = function.right( function.length()-1 );
	kLineEditYFunction->setText( function );
	checkBoxHide->setChecked( m_parser->fktext[ m_id ].f_mode == 0 );
	if (  m_parser->fktext[ m_id ].dmin != m_parser->fktext[ m_id ].dmax )
	{
		checkBoxRange->setChecked( true );
		min->setText( m_parser->fktext[ m_id ].str_dmin );
		max->setText( m_parser->fktext[ m_id ].str_dmax );
	}
	else
		checkBoxRange->setChecked( false );
	kIntNumInputLineWidth->setValue( m_parser->fktext[ m_id ].linewidth );
	kColorButtonColor->setColor( m_parser->fktext[ m_id ].color );
}

void KEditPolar::accept()
{
	QString f_str = kLineEditYFunction->text();
	int id;
        Parser::Ufkt *tmp_ufkt;
	if( m_id != -1 )  //when editing a function: 
	{
                int const ix = m_parser->ixValue(m_id);
                if ( ix == -1) //The function could have been deleted
                {
                        KMessageBox::error(this,i18n("Function could not be found"));
                        return;
                }
                tmp_ufkt =  &m_parser->ufkt[ix];
                id = m_id; //use the right function-id
		m_parser->fixFunctionName(f_str, XParser::Polar, id);
		f_str.prepend("r");
		QString old_fstr = m_parser->ufkt[id].fstr;
		m_parser->ufkt[id].fstr = f_str;
		m_parser->reparse(id); //reparse the funcion
		if ( m_parser->errmsg() != 0)
		{
			m_parser->ufkt[id].fstr = old_fstr;
			m_parser->reparse(id); 
			this->raise();
			kLineEditYFunction->setFocus();
			kLineEditYFunction->selectAll();
			return;
		}
	}
	else
	{
		m_parser->fixFunctionName(f_str, XParser::Polar);
		f_str.prepend("r");
		id = m_parser->addfkt(f_str );
		kdDebug() << "id: " << id << endl;
                
                if( id == -1 ) 
                {
                        m_parser->errmsg();
                        this->raise();
                        kLineEditYFunction->setFocus();
                        kLineEditYFunction->selectAll();
                        return;
                }
                tmp_ufkt =  &m_parser->ufkt.last();
	}

	XParser::FktExt tmp_fktext; //all settings are saved here until we know that no errors have appeared
	tmp_fktext.extstr = f_str;
		
	if( checkBoxHide->isChecked() )
		tmp_fktext.f_mode = 0;
	else
		tmp_fktext.f_mode = 1;
	
	if( checkBoxRange->isChecked() )
	{
		tmp_fktext.str_dmin = min->text();
		tmp_fktext.dmin = m_parser->eval( min->text() );
		if ( m_parser->errmsg() )
		{
			min->setFocus();
			min->selectAll();
			if( m_id == -1 ) m_parser->Parser::delfkt( tmp_ufkt );
			return;
		}
		tmp_fktext.str_dmax = max->text();
		tmp_fktext.dmax = m_parser->eval( max->text() );
		if ( m_parser->errmsg())
		{
			max->setFocus();
			max->selectAll();
			if( m_id == -1 ) m_parser->Parser::delfkt( tmp_ufkt );
			return;
		}
		if ( tmp_fktext.dmin >=  tmp_fktext.dmax)
		{
			KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
			min->setFocus();
			min->selectAll();
			if( m_id == -1 ) m_parser->Parser::delfkt( tmp_ufkt );
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
	tmp_fktext.f1_mode = 0;
	tmp_fktext.f2_mode = 0;
	tmp_fktext.integral_mode = 0;
	tmp_fktext.linewidth = kIntNumInputLineWidth->value();
	tmp_fktext.color = kColorButtonColor->color().rgb();
	tmp_fktext.use_slider = -1;
	
	//tmp_fktext.color0 = m_parser->fktext[id].color0; ///Should we change the default color?
        
        //save all settings in the function now when we now no errors have appeared
        if( m_id == -1 )
        {
                tmp_fktext.id = tmp_ufkt->id;
                m_parser->fktext.append(tmp_fktext);
        }
        else
        {
                tmp_fktext.id = id;
                m_parser->fktext[id] = tmp_fktext; 
        }
	
        
        kLineEditYFunction->setText(f_str); //update the function name in FktDlg
	// call inherited method
	QEditPolar::accept();
}

const QString KEditPolar::functionItem()
{
	return kLineEditYFunction->text();
}

void KEditPolar::slotHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}