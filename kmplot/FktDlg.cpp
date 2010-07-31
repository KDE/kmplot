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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

// Qt includes
// #include <tqpushbutton.h>
// #include <tqpopupmenu.h>

// KDE includes
#include <kapplication.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <tqcstring.h>
#include <tqcursor.h>
#include <tqheader.h>

// locale includes
#include "FktDlg.h"
#include "FktDlg.moc"
#include "editfunction.h"
#include "keditparametric.h"
#include "keditpolar.h"
#include "MainDlg.h"
#include "parseriface.h"
#include "xparser.h"

#include <kdebug.h>

FktDlg::FktDlg( TQWidget* parent, View *view ) : FktDlgData( parent, "editPlots" ), m_view(view)
{
	connect( cmdCopyFunction, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotCopyFunction()) );
	connect( cmdMoveFunction, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotMoveFunction()) );
	connect( lb_fktliste, TQT_SIGNAL( doubleClicked(TQListViewItem *, const TQPoint &, int)), this, TQT_SLOT(lb_fktliste_doubleClicked(TQListViewItem *, const TQPoint &, int)) );
	connect( lb_fktliste, TQT_SIGNAL( clicked(TQListViewItem *)), this, TQT_SLOT(lb_fktliste_clicked(TQListViewItem *)));
	connect( lb_fktliste, TQT_SIGNAL( spacePressed(TQListViewItem *)), this, TQT_SLOT(lb_fktliste_spacePressed(TQListViewItem *)));

	lb_fktliste->addColumn( "" );
	lb_fktliste->header()->hide();
	lb_fktliste->setResizeMode( TQListView::LastColumn );
}

FktDlg::~FktDlg()
{
}

void FktDlg::slotDelete()
{
	if ( lb_fktliste->currentItem() == 0)
		return;
	TQCheckListItem *currentItem = dynamic_cast<TQCheckListItem *>(lb_fktliste->currentItem());
	if( currentItem->text()[0] == 'x' )
	{
    // Delete pair of parametric function
		int const id = getParamId( currentItem->text());
		if ( id == -1)
			return;
		if ( !m_view->parser()->delfkt(id))
			return;
	}
	else
	{
    // only one function to be deleted
		if (!m_view->parser()->delfkt( getId( currentItem->text()) ) )
			return;
	}
	lb_fktliste->takeItem( currentItem );
	changed = true;
	updateView();
	if ( lb_fktliste->childCount()==0 )
		PushButtonDel->setEnabled(false);
}

void FktDlg::lb_fktliste_doubleClicked( TQListViewItem *, const TQPoint &, int )
{
	if ( mapFromGlobal(TQCursor::pos()).x() <= 40 )
		return;
	slotEdit();
}

void FktDlg::lb_fktliste_spacePressed( TQListViewItem *item)
{
	if ( !item )
		return;
	TQCheckListItem *currentItem = dynamic_cast<TQCheckListItem *>(item);
	int id;
	if ( currentItem->text()[0] == 'x' )
		id = getParamId(currentItem->text());
	else
		id = getId(currentItem->text());
	Ufkt *function = &m_view->parser()->ufkt[ m_view->parser()->ixValue(id) ];
	if ( id==-1 )
		return;
	
	function->f_mode=currentItem->isOn();
	updateView();
}

void FktDlg::lb_fktliste_clicked( TQListViewItem * item )
{
	if ( mapFromGlobal(TQCursor::pos()).x() > 40 )
		return;
	lb_fktliste_spacePressed(item);
}


void FktDlg::slotEdit()
{
	TQCheckListItem *currentItem = dynamic_cast<TQCheckListItem *>(lb_fktliste->currentItem());
	if ( currentItem == 0 )
	{
		PushButtonEdit->setEnabled(false);
		return;
	}
	int const id = getId( currentItem->text().section( ";", 0, 0) ) ;
	
	// find out the function type
	char const prefix = m_view->parser()->ufkt[ m_view->parser()->ixValue(id) ].fstr.at(0).latin1();
	
	if ( prefix == 'r')
		slotEditPolar( id );
	else if ( prefix == 'x')
		slotEditParametric( id, m_view->parser()->ixValue(getId( currentItem->text().section( ";", 1, 1) )));
	else
		slotEditFunction( id );
}

int FktDlg::getId( const TQString &f_str )
{
	for( TQValueVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
	{
		if ( it->fstr == f_str )
			return it->id;
	}
	return -1;
}

int FktDlg::getParamId( const TQString &f_str)
{
	TQString const fname = f_str.section( "(", 0, 0 );
	for( TQValueVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
	{
		if ( it->fname == fname )
			return it->id;
	}
	return -1;
}

void FktDlg::updateView()
{
	m_view->drawPlot();
}

void FktDlg::slotHasSelection()
{
	bool const has_selection = !( lb_fktliste->currentItem() == 0 );
	PushButtonEdit->setEnabled( has_selection );
	PushButtonDel->setEnabled( has_selection );
	cmdCopyFunction->setEnabled( has_selection );
	cmdMoveFunction->setEnabled( has_selection );
}

void FktDlg::slotEditFunction( int id )
{
	EditFunction* editFunction = new EditFunction( m_view->parser(), this );
	if ( id==-1 ) editFunction->setCaption(i18n( "New Function Plot" ));
	else editFunction->setCaption(i18n( "Edit Function Plot" ));
	editFunction->initDialog( id );
	if( editFunction->exec() == TQDialog::Accepted )
	{
		Ufkt *function = editFunction->functionItem();
		TQCheckListItem *item;
		if( id == -1 ) //a new function
			item = new TQCheckListItem(lb_fktliste, function->fstr, TQCheckListItem::CheckBox); //a new function
		else //change a function
		{
			item = dynamic_cast<TQCheckListItem *>(lb_fktliste->currentItem());
			item->setText(0, function->fstr );
		}
		item->setOn(function->f_mode);
		lb_fktliste->sort();
		changed = true;
		updateView();
	}
}

void FktDlg::slotEditParametric( int x_id, int y_id)
{
	KEditParametric* editParametric = new KEditParametric( m_view->parser(), this );
	if ( x_id==-1 && y_id==-1)
		editParametric->setCaption(i18n( "New Parametric Plot" ));
	editParametric->initDialog( x_id, y_id );
	if( editParametric->exec() == TQDialog::Accepted )
	{
		Ufkt *function_y = editParametric->functionItem();
		Ufkt *function_x = &m_view->parser()->ufkt[m_view->parser()->ixValue(function_y->id - 1)]; //get the x-function
		TQCheckListItem *item;
		if( x_id == -1 ) //a new function
		{
			item = new TQCheckListItem(lb_fktliste, function_x->fstr+";"+function_y->fstr, TQCheckListItem::CheckBox);
		}
		else //change a function
		{
			item = dynamic_cast<TQCheckListItem *>(lb_fktliste->currentItem());
			item->setText(0, function_x->fstr+";"+function_y->fstr );
		}
		item->setOn(function_y->f_mode);
		lb_fktliste->sort();
		changed = true;
		updateView();
	}
}

void FktDlg::slotEditPolar( int id )
{
	KEditPolar* editPolar = new KEditPolar( m_view->parser(), this );
	if ( id==-1) editPolar->setCaption(i18n( "New Polar Plot" ));
	editPolar->initDialog( id );
	if( editPolar->exec() == TQDialog::Accepted )
	{
		Ufkt *function = editPolar->functionItem();
		TQCheckListItem *item;
		if( id == -1 ) // a new function
		{
			item = new TQCheckListItem(lb_fktliste, function->fstr, TQCheckListItem::CheckBox);
		}
		else //change a function
		{
			item = dynamic_cast<TQCheckListItem *>(lb_fktliste->currentItem());
			item->setText(0, function->fstr );
		}
		item->setOn(function->f_mode);
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
	for( TQValueVector<Ufkt>::iterator it = m_view->parser()->ufkt.begin(); it != m_view->parser()->ufkt.end(); ++it)
	{
		if( it->fname.isEmpty() || it->fstr[0] == 'y' ) continue;
		TQCheckListItem *item;
		if( it->fstr[0] == 'x' )
		{
			TQString y = it->fstr;
			++it;
			item = new TQCheckListItem( lb_fktliste,  y + ";" + it->fstr, TQCheckListItem::CheckBox );
		}
		else
			item = new TQCheckListItem(lb_fktliste, it->fstr, TQCheckListItem::CheckBox);
		item->setOn(it->f_mode);
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

void FktDlg::showEvent ( TQShowEvent * )
{
	changed = false;
}

void FktDlg::slotCopyFunction()
{
	if ( lb_fktliste->currentItem() == 0)
	{
		cmdCopyFunction->setEnabled(false);
		return;
	}
	TQCheckListItem *currentItem = dynamic_cast<TQCheckListItem *>(lb_fktliste->currentItem());
	TQString const fstr = currentItem->text();
	m_view->parser()->sendFunction(getId(fstr));
}

void FktDlg::slotMoveFunction()
{
	if ( lb_fktliste->currentItem() == 0)
	{
		cmdCopyFunction->setEnabled(false);
		return;
	}
	TQCheckListItem *currentItem = dynamic_cast<TQCheckListItem *>(lb_fktliste->currentItem());
	TQString const fstr = currentItem->text();
	if ( !m_view->parser()->sendFunction(getId( currentItem->text()) ) )
		return;
	slotDelete();
}
