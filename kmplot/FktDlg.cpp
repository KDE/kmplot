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
// #include <qpushbutton.h>
// #include <qpopupmenu.h>

// KDE includes
#include <kpushbutton.h>
#include <klineedit.h>

// locale includes
#include "FktDlg.h"
#include "FktDlg.moc"
#include "MainDlg.h"
#include "keditfunction.h"

#include <kdebug.h>

#define Inherited FktDlgData

FktDlg::FktDlg( QWidget* parent, const char* name ) : Inherited( parent, name )
{
/*	QPopupMenu *menu_types = new QPopupMenu( this );
	menu_types->insertItem( i18n( "Function Plot" ), this, SLOT( onEditFunction() ) );
	menu_types->insertItem( i18n( "Parametric Plot" ), this, SLOT( onEditParametric() ) );
	menu_types->insertItem( i18n( "Polar Plot" ), this, SLOT( onEditPolar() ) );
	PushButtonNew->setPopup( menu_types );*/
	
	editFunction = 0;
}

FktDlg::~FktDlg()
{
}

// Slots

// void FktDlg::onnew()
// {
// 	int ix;
// 	char c0;
// 	QString fname, fstr, str;
// 
// //	fstr = le_fktstr->text();
// 	if ( !fstr.isEmpty() )
// 	{
// 		// left from semicolon is function equation
// 		int i = fstr.find( ';' );
// 
// 		if ( i == -1 )
// 			str = fstr;
// 		else
// 			str = fstr.left( i );
// 
// 		// test the function equation syntax
// 		ix = ps.addfkt( str );
// 		if ( ix == -1 )
// 		{
// 			ps.errmsg();
// 			errflg = 1;
// 			return ;
// 		}
// 
// 		// handle the extensions
// 		ps.fktext[ ix ].extstr = fstr;
// 		// test the extension syntax
// 		if ( ps.getext( ix ) == -1 )
// 		{
// 			errflg = 1;
// 			return ;
// 		}
// 
// 		// handle parametric functions: xf(t), yf(t)
// 		if ( ( c0 = fstr[ 0 ].latin1() ) == 'x' )
// 		{
// 			ps.getfkt( ix, fname, str );
// 			fname[ 0 ] = 'y';
// 			if ( ps.getfix( fname ) == -1 )
// 			{
// 				int p;
// 
// 				lb_fktliste->insertItem( fstr );
// 				// fktidx.append( lb_fktliste->currentItem() );
// 				p = fstr.find( '=' );
// 				fstr = fstr.left( p + 1 );
// 				fstr[ 0 ] = 'y';
// /*				le_fktstr->setText( fstr );
// 				le_fktstr->setFocus();
// 				le_fktstr->deselect();
// */				return ;
// 			}
// 		}
// 		else if ( c0 == 'y' )
// 		{
// 			if ( ps.getfkt( ix, fname, str ) != -1 )
// 			{
// 				fname[ 0 ] = 'x';
// 				ix = ps.getfix( fname );
// 			}
// 		}
// 		lb_fktliste->insertItem( fstr );
// //		le_fktstr->clear();
// 	}
// 	errflg = 0;
// 	updateView();
// }

void FktDlg::ondelete()
{
	int ix, num;

	if ( ( num = lb_fktliste->currentItem() ) == -1 )
		return ;

	ix = getIx( lb_fktliste->text( num ) );
	chflg = 1;
	ps.delfkt( ix );
	lb_fktliste->removeItem( num );
	updateView();
}

void FktDlg::onedit()
{
	int num = lb_fktliste->currentItem();
	int index = getIx( lb_fktliste->text( num ).section( ";", 0, 0) );
	
	if( !editFunction ) editFunction = new KEditFunction( &ps, this );
	
	// find out the function type
	char prefix = ps.fktext[ index ].extstr.at(0).latin1();
	
	switch( prefix )
	{
		case 'r':
			onEditPolar( index, num );
			break;
		case 'x':
			onEditParametric( index, getIx( lb_fktliste->text( num ).section( ";", 1, 1) ), num );
			break;
		default:
			onEditFunction( index, num );
	}
	updateView();
}

int FktDlg::getIx( const QString f_str )
{
	QString fname;
	QString fstr;
	for ( int ix = 0; ix < ps.ufanz; ++ix )
	{
		if ( ps.getfkt( ix, fname, fstr ) == -1 )
			continue;

		if ( ps.fktext[ ix ].extstr == f_str )
			return ix;
	}
	return -1;
}

void FktDlg::updateView()
{
	( ( MainDlg* ) parentWidget() ) ->view->update();
}

void FktDlg::onHasSelection()
{
	bool has_selection = !( lb_fktliste->currentItem() == -1 );
	PushButtonEdit->setEnabled( has_selection );
	PushButtonDel->setEnabled( has_selection );
}

void FktDlg::onEditFunction( int index, int num )
{
	if( !editFunction ) editFunction = new KEditFunction( &ps, this );
	editFunction->initDialog( KEditFunction::Function, index );
	if( editFunction->exec() == QDialog::Accepted )
	{
		if( index == -1 ) lb_fktliste->insertItem( editFunction->yFunction() );
		else lb_fktliste->changeItem( editFunction->yFunction(), num );
	}
}

void FktDlg::onEditParametric( int x_index, int y_index, int num )
{
	if( !editFunction ) editFunction = new KEditFunction( &ps, this );
	editFunction->initDialog( KEditFunction::Parametric, x_index, y_index );
	if( editFunction->exec() == QDialog::Accepted )
	{
		if( x_index == -1 ) 
			lb_fktliste->insertItem( editFunction->xFunction() + ";" + editFunction->yFunction() );
		else lb_fktliste->changeItem( editFunction->xFunction() + ";" + editFunction->yFunction(), num );
	}
}

void FktDlg::onEditPolar( int index, int num )
{
	if( !editFunction ) editFunction = new KEditFunction( &ps, this );
	editFunction->initDialog( KEditFunction::Polar, index );
	if( editFunction->exec() == QDialog::Accepted )
	{
		if( index == -1 ) lb_fktliste->insertItem( editFunction->yFunction() );
		else lb_fktliste->changeItem( editFunction->yFunction(), num );
	}
}

void FktDlg::onNewFunction()
{
	onEditFunction();
}

void FktDlg::onNewParametric()
{
	onEditParametric();
}

void FktDlg::onNewPolar()
{
	onEditPolar();
}

void FktDlg::fillList()
{
	int index;
	QString fname, fstr;

	lb_fktliste->clear();

	// adding all yet added functions
	for ( index = 0; index < ps.ufanz; ++index )
	{
		if ( ps.getfkt( index, fname, fstr ) == -1 ) continue;
		if( fname[0] == 'y' ) continue;
		if( fname[0] == 'x' )
		{
			QString y_name( fname );
			y_name[0] = 'y';
			int y_index = ps.getfix( y_name );
			if( y_index == -1 ) continue;
			lb_fktliste->insertItem( ps.fktext[ index ].extstr + ";" + ps.fktext[ y_index ].extstr );
		}
		else lb_fktliste->insertItem( ps.fktext[ index ].extstr );
	}
}
