/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
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
// #include <qpushbutton.h>
// #include <qpopupmenu.h>

// KDE includes
#include <kapplication.h>
#include <klocale.h>
#include <kpushbutton.h>

// locale includes
#include "FktDlg.h"
#include "FktDlg.moc"
#include "editfunction.h"
#include "keditparametric.h"
#include "keditpolar.h"
#include "MainDlg.h"
#include "xparser.h"

#include <kdebug.h>

#define Inherited FktDlgData

FktDlg::FktDlg( QWidget* parent, XParser* parser ) : Inherited( parent, "editPlots" ), m_parser(parser)
{
}

FktDlg::~FktDlg()
{
}

void FktDlg::slotDelete()
{
	if ( lb_fktliste->currentItem()== -1)
	{
		PushButtonDel->setEnabled(false);
		return;
	}
	int num;
	if ( ( num = lb_fktliste->currentItem() ) == -1 ) return ;
	
	if( lb_fktliste->text( num )[0] == 'x' )
	{
		// Delete pair of parametric function
		int const ix = m_parser->ixValue( getParamId( lb_fktliste->text( num ) ) );
                if ( ix == -1)
                        return;
                m_parser->delfkt( ix+1 );
                m_parser->delfkt( ix );
	}
	else 
	{
		// only one function to be deleted
		m_parser->delfkt( m_parser->ixValue(getId( lb_fktliste->text( num ))) );
                
	}
	lb_fktliste->removeItem( num );
	changed = true;
	updateView();
}

void FktDlg::slotEdit()
{
	if ( lb_fktliste->currentItem()==-1 )
	{
		PushButtonEdit->setEnabled(false);
		return;
	}
	int const num = lb_fktliste->currentItem();
	int const id = getId( lb_fktliste->currentText().section( ";", 0, 0) ) ;
	
	// find out the function type
	char const prefix = m_parser->ufkt[ m_parser->ixValue(id) ].extstr.at(0).latin1();
	
	if ( prefix == 'r')
		slotEditPolar( id, num );
	else if ( prefix == 'x')
		slotEditParametric( id, m_parser->ixValue(getId( lb_fktliste->text( num ).section( ";", 1, 1) )), num );
	else
		slotEditFunction( id, num );
}

int FktDlg::getId( const QString &f_str )
{
        for( QValueVector<Ufkt>::iterator it =  m_parser->ufkt.begin(); it !=  m_parser->ufkt.end(); ++it)
	{
		if ( it->extstr == f_str )
			return it->id;
	}
	return -1;
}

int FktDlg::getParamId( const QString &f_str)
{
	QString const fname = f_str.section( "(", 0, 0 );
        for( QValueVector<Ufkt>::iterator it =  m_parser->ufkt.begin(); it !=  m_parser->ufkt.end(); ++it)
        {
                if ( it->fname == fname )
                        return it->id;
        }
        return -1;
}

void FktDlg::updateView()
{
	( ( MainDlg* ) parentWidget() ) ->view->drawPlot();
}

void FktDlg::slotHasSelection()
{
	bool has_selection = !( lb_fktliste->currentItem() == -1 );
	PushButtonEdit->setEnabled( has_selection );
	PushButtonDel->setEnabled( has_selection );
}

void FktDlg::slotEditFunction( int id, int num )
{
	EditFunction* editFunction = new EditFunction( m_parser, this );
	if ( id==-1&&num==-1) editFunction->setCaption(i18n( "New Function Plot" ));
	else editFunction->setCaption(i18n( "Edit Function Plot" ));
	editFunction->initDialog( id );
	if( editFunction->exec() == QDialog::Accepted )
	{
		if( id == -1 ) lb_fktliste->insertItem( editFunction->functionItem() ); //a new function
		else lb_fktliste->changeItem( editFunction->functionItem(), num ); //changed a function
		lb_fktliste->sort();
                changed = true;
		updateView();
	}
}

void FktDlg::slotEditParametric( int x_id, int y_id, int num )
{
	KEditParametric* editParametric = new KEditParametric( m_parser, this );
	if ( x_id==-1&&y_id==-1&&num==-1) editParametric->setCaption(i18n( "New Parametric Plot" ));
	editParametric->initDialog( x_id, y_id );
	if( editParametric->exec() == QDialog::Accepted )
	{
		if( x_id == -1 ) lb_fktliste->insertItem( editParametric->functionItem() ); //a new function
		else lb_fktliste->changeItem( editParametric->functionItem(), num ); //changed a function
		lb_fktliste->sort();
                changed = true;
		updateView();
	}
}

void FktDlg::slotEditPolar( int id, int num )
{
	KEditPolar* editPolar = new KEditPolar( m_parser, this );
	if ( id==-1&&num==-1) editPolar->setCaption(i18n( "New Polar Plot" ));
	editPolar->initDialog( id );
	if( editPolar->exec() == QDialog::Accepted )
	{
		if( id == -1 ) lb_fktliste->insertItem( editPolar->functionItem() ); //a new function
		else lb_fktliste->changeItem( editPolar->functionItem(), num ); //changed a function
		lb_fktliste->sort();
                changed = true;
		updateView();
	}
}

void FktDlg::slotNewFunction()
{
	slotEditFunction();
}

void FktDlg::slotNewParametric()
{
	slotEditParametric();
}

void FktDlg::slotNewPolar()
{
	slotEditPolar();
}

void FktDlg::getPlots()
{
	lb_fktliste->clear();

	// adding all yet added functions
        for( QValueVector<Ufkt>::iterator it = m_parser->ufkt.begin(); it != m_parser->ufkt.end(); ++it)
	{
                if( it->fname.isEmpty() || it->extstr[0] == 'y' ) continue;
		if( it->extstr[0] == 'x' )
		{
                        QString y = it->extstr;
                        ++it;
			lb_fktliste->insertItem( y + ";" + it->extstr );
		}
		else lb_fktliste->insertItem( it->extstr );
	}
        lb_fktliste->sort();
}

void  FktDlg::slotHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}

bool FktDlg::isChanged()
{
	return changed;
}

void FktDlg::showEvent ( QShowEvent * )
{
	changed = false;
}
