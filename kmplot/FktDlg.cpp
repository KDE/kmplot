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
	int ix, num;
	if ( ( num = lb_fktliste->currentItem() ) == -1 ) return ;
	
	if( lb_fktliste->text( num )[0] == 'x' )
	{
		// Delete pair of parametric function
		int i, j;
		getParamIx( lb_fktliste->text( num ), i, j );
		m_parser->delfkt( i );
		m_parser->delfkt( j );
	}
	else 
	{
		// only one function to be deleted
		ix = getIx( lb_fktliste->text( num ) );
		m_parser->delfkt( ix );
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
	int num = lb_fktliste->currentItem();
	int index = getIx( lb_fktliste->text( num ).section( ";", 0, 0) );
	
	// find out the function type
	char prefix = m_parser->fktext[ index ].extstr.at(0).latin1();
	
	if ( prefix == 'r')
		slotEditPolar( index, num );
	else if ( prefix == 'x')
		slotEditParametric( index, getIx( lb_fktliste->text( num ).section( ";", 1, 1) ), num );
	else
		slotEditFunction( index, num );
}

int FktDlg::getIx( const QString f_str )
{
	QString fname;
	QString fstr;
	for ( int ix = 0; ix < m_parser->ufanz; ++ix )
	{
		if ( m_parser->getfkt( ix, fname, fstr ) == -1 )
			continue;

		if ( m_parser->fktext[ ix ].extstr == f_str )
			return ix;
	}
	return -1;
}

void FktDlg::getParamIx( const QString f_str, int &index1, int &index2 )
{
	QString fname = f_str.section( "(", 0, 0 );
	index1 = m_parser->getfix( fname );
	if( fname[0] == 'x' ) fname[0] = 'y';
	else fname[0] = 'x';
	index2 = m_parser->getfix( fname );
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

void FktDlg::slotEditFunction( int index, int num )
{
	EditFunction* editFunction = new EditFunction( m_parser, this );
	if ( index==-1&&num==-1) editFunction->setCaption(i18n( "New Function Plot" ));
	else editFunction->setCaption(i18n( "Edit Function Plot" ));
	editFunction->initDialog( index );
	if( editFunction->exec() == QDialog::Accepted )
	{
		if( index == -1 ) lb_fktliste->insertItem( editFunction->functionItem() ); //a new function
		else lb_fktliste->changeItem( editFunction->functionItem(), num ); //changed a function
		changed = true;
		updateView();
	}
}

void FktDlg::slotEditParametric( int x_index, int y_index, int num )
{
	KEditParametric* editParametric = new KEditParametric( m_parser, this );
	if ( x_index==-1&&y_index==-1&&num==-1) editParametric->setCaption(i18n( "New Parametric Plot" ));
	editParametric->initDialog( x_index, y_index );
	if( editParametric->exec() == QDialog::Accepted )
	{
		if( x_index == -1 ) lb_fktliste->insertItem( editParametric->functionItem() ); //a new function
		else lb_fktliste->changeItem( editParametric->functionItem(), num ); //changed a function
		changed = true;
		updateView();
	}
}

void FktDlg::slotEditPolar( int index, int num )
{
	KEditPolar* editPolar = new KEditPolar( m_parser, this );
	if ( index==-1&&num==-1) editPolar->setCaption(i18n( "New Polar Plot" ));
	editPolar->initDialog( index );
	if( editPolar->exec() == QDialog::Accepted )
	{
		if( index == -1 ) lb_fktliste->insertItem( editPolar->functionItem() ); //a new function
		else lb_fktliste->changeItem( editPolar->functionItem(), num ); //changed a function
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
	int index;
	QString fname, fstr;

	lb_fktliste->clear();

	// adding all yet added functions
	for ( index = 0; index < m_parser->ufanz; ++index )
	{
		if ( m_parser->getfkt( index, fname, fstr ) == -1 ) continue;
		if( fname[0] == 'y' ) continue;
		if( fname[0] == 'x' )
		{
			QString y_name( fname );
			y_name[0] = 'y';
			int y_index = m_parser->getfix( y_name );
			if( y_index == -1 ) continue;
			lb_fktliste->insertItem( m_parser->fktext[ index ].extstr + ";" + m_parser->fktext[ y_index ].extstr );
		}
		else lb_fktliste->insertItem( m_parser->fktext[ index ].extstr );
	}
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
