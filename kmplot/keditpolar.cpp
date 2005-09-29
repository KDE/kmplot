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
	connect( customMinRange, SIGNAL ( toggled(bool) ), this, SLOT( customMinRange_toggled(bool) ) );
	connect( customMaxRange, SIGNAL ( toggled(bool) ), this, SLOT( customMaxRange_toggled(bool) ) );
	m_updatedfunction = 0;
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
	customMinRange->setChecked( false );
	customMaxRange->setChecked(false);
	min->clear();
	max->clear();
	kIntNumInputLineWidth->setValue( m_parser->linewidth0 );
	kColorButtonColor->setColor( m_parser->defaultColor(m_parser->getNextIndex() ) );
}

void KEditPolar::setWidgets()
{
        Ufkt *ufkt = &m_parser->ufkt[ m_parser->ixValue(m_id) ];
	QString function = ufkt->fstr;
	function = function.right( function.length()-1 );
	kLineEditYFunction->setText( function );
	checkBoxHide->setChecked( !ufkt->f_mode);
	if (ufkt->usecustomxmin)
	{
		customMinRange->setChecked(true);
		min->setText( ufkt->str_dmin );
	}
	else
		customMinRange->setChecked(false);
	
	if (ufkt->usecustomxmin)
	{
		customMaxRange->setChecked(true);
		max->setText( ufkt->str_dmax );
	}
	else
		customMaxRange->setChecked(false);
	
	kIntNumInputLineWidth->setValue( ufkt->linewidth );
	kColorButtonColor->setColor( ufkt->color );
}

void KEditPolar::accept()
{
	QString f_str = /*"r" + */kLineEditYFunction->text();

	if ( m_id!=-1 )
		m_parser->fixFunctionName(f_str, XParser::Polar, m_id);
	else
		m_parser->fixFunctionName(f_str, XParser::Polar);
	Ufkt tmp_ufkt;  //all settings are saved here until we know that no errors have appeared

	tmp_ufkt.f_mode = !checkBoxHide->isChecked();
	
	if( customMinRange->isChecked() )
	{
		tmp_ufkt.usecustomxmin = true;
		tmp_ufkt.str_dmin = min->text();
		tmp_ufkt.dmin = m_parser->eval( min->text() );
		if ( m_parser->parserError() )
		{
			min->setFocus();
			min->selectAll();
			return;
		}
	}
	else
		tmp_ufkt.usecustomxmin = false;
	if( customMaxRange->isChecked() )
	{
		tmp_ufkt.usecustomxmax = true;
		tmp_ufkt.str_dmax = max->text();
		tmp_ufkt.dmax = m_parser->eval( max->text() );
		if ( m_parser->parserError())
		{
			max->setFocus();
			max->selectAll();
			return;
		}
		if ( tmp_ufkt.usecustomxmin && tmp_ufkt.dmin >=  tmp_ufkt.dmax)
		{
			KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
			min->setFocus();
			min->selectAll();
			return;
		}
	}
	else
		tmp_ufkt.usecustomxmax = false;
	
	tmp_ufkt.f1_mode = 0;
	tmp_ufkt.f2_mode = 0;
	tmp_ufkt.integral_mode = 0;
	tmp_ufkt.linewidth = kIntNumInputLineWidth->value();
  tmp_ufkt.color = kColorButtonColor->color().rgb();
	tmp_ufkt.use_slider = -1;
        
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
                added_ufkt->fstr = f_str;
                m_parser->reparse(added_ufkt); //reparse the funcion
                if ( m_parser->parserError() != 0)
                {
                        added_ufkt->fstr = old_fstr;
                        m_parser->reparse(added_ufkt);
                        raise();
                        kLineEditYFunction->setFocus();
                        kLineEditYFunction->selectAll();
                        return;
                }
        }
        else
        {
                int const id = m_parser->addfkt(f_str );
                kdDebug() << "id: " << id << endl;
                if( id == -1 ) 
                {
                        m_parser->parserError();
                        raise();
                        kLineEditYFunction->setFocus();
                        kLineEditYFunction->selectAll();
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
	QEditPolar::accept();
}

Ufkt *KEditPolar::functionItem()
{
	return m_updatedfunction;
}

void KEditPolar::slotHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}

void KEditPolar::customMinRange_toggled(bool status)
{
	if (status)
		min->setEnabled(true);
	else
		min->setEnabled(false);
}

void KEditPolar::customMaxRange_toggled(bool status)
{
	if (status)
		max->setEnabled(true);
	else
		max->setEnabled(false);
}
