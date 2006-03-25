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
// #include <qpushbutton.h>
// #include <qpopupmenu.h>

// KDE includes
#include <kapplication.h>
#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <qcursor.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QShowEvent>

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
#include <ktoolinvocation.h>

FktDlg::FktDlg( QWidget* parent, View *view )
	: KDialog( parent, i18n("Edit Plots"), Ok|Cancel|Help ),
	  m_view(view)
{
	m_mainWidget = new FktDlgData( this );
	setMainWidget( m_mainWidget );
	
	connect( m_mainWidget->cmdCopyFunction, SIGNAL( clicked() ), this, SLOT( slotCopyFunction()) );
	connect( m_mainWidget->cmdMoveFunction, SIGNAL( clicked() ), this, SLOT( slotMoveFunction()) );
	connect( m_mainWidget->lb_fktliste, SIGNAL( itemDoubleClicked(QListWidgetItem *)), this, SLOT(lb_fktliste_doubleClicked(QListWidgetItem *)) );
	connect( m_mainWidget->lb_fktliste, SIGNAL( itemClicked(QListWidgetItem *)), this, SLOT(lb_fktliste_clicked(QListWidgetItem *)));
	
	connect(m_mainWidget->PushButtonDel, SIGNAL(clicked()), this, SLOT(slotDelete()));
	connect(m_mainWidget->lb_fktliste, SIGNAL(itemSelectionChanged()), this, SLOT(slotHasSelection()));
// 	connect(PushButtonOk, SIGNAL(clicked()), this, SLOT(accept()));
// 	connect(pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(m_mainWidget->PushButtonEdit, SIGNAL(clicked()), this, SLOT(slotEdit()));
	connect(m_mainWidget->PushButtonNewFunction, SIGNAL(clicked()), this, SLOT(slotNewFunction()));
	connect(m_mainWidget->PushButtonNewParametric, SIGNAL(clicked()), this, SLOT(slotNewParametric()));
	connect(m_mainWidget->PushButtonNewPolar, SIGNAL(clicked()), this, SLOT(slotNewPolar()));
// 	connect(pushButtonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
}

FktDlg::~FktDlg()
{
}

void FktDlg::slotDelete()
{
	if ( m_mainWidget->lb_fktliste->currentItem() == 0)
		return;
	QListWidgetItem *currentItem = m_mainWidget->lb_fktliste->currentItem();
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
	m_mainWidget->lb_fktliste->takeItem( m_mainWidget->lb_fktliste->currentRow() );
	changed = true;
	updateView();
	slotHasSelection();
}


void FktDlg::lb_fktliste_doubleClicked( QListWidgetItem * )
{
	slotEdit();
}


void FktDlg::lb_fktliste_clicked( QListWidgetItem * )
{
	// use a single shot as the check state of the item isn't updated before emitting the clicked signal
	QTimer::singleShot( 0, this, SLOT( checkCurrentItemCheckState() ) );
}


void FktDlg::checkCurrentItemCheckState()
{
	QListWidgetItem * item = m_mainWidget->lb_fktliste->currentItem();
	
	int id;
	if ( item->text()[0] == 'x' )
		id = getParamId(item->text());
	else
		id = getId(item->text());
	Ufkt *function = &m_view->parser()->ufkt[ m_view->parser()->ixValue(id) ];
	if ( id==-1 )
		return;
	
	function->f_mode = (item->checkState() & Qt::Checked);
	updateView();
}


void FktDlg::slotEdit()
{
	QListWidgetItem * currentItem = m_mainWidget->lb_fktliste->currentItem();
	if ( currentItem == 0 )
	{
		m_mainWidget->PushButtonEdit->setEnabled(false);
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

int FktDlg::getId( const QString &f_str )
{
	for( QVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
	{
		if ( it->fstr == f_str )
			return it->id;
	}
	return -1;
}

int FktDlg::getParamId( const QString &f_str)
{
	QString const fname = f_str.section( "(", 0, 0 );
	for( QVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
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
	bool const has_selection = !m_mainWidget->lb_fktliste->selectedItems().isEmpty();
	m_mainWidget->PushButtonEdit->setEnabled( has_selection );
	m_mainWidget->PushButtonDel->setEnabled( has_selection );
	m_mainWidget->cmdCopyFunction->setEnabled( has_selection );
	m_mainWidget->cmdMoveFunction->setEnabled( has_selection );
}

void FktDlg::slotEditFunction( int id )
{
	EditFunction* editFunction = new EditFunction( m_view->parser(), this );
	if ( id==-1 ) editFunction->setCaption(i18n( "New Function Plot" ));
	else editFunction->setCaption(i18n( "Edit Function Plot" ));
	editFunction->initDialog( id );
	if( editFunction->exec() == QDialog::Accepted )
	{
		Ufkt *function = editFunction->functionItem();
		QListWidgetItem *item;
		if( id == -1 ) //a new function
			item = new QListWidgetItem( function->fstr, m_mainWidget->lb_fktliste ); //a new function
		else //change a function
		{
			item = m_mainWidget->lb_fktliste->currentItem();
			item->setText( function->fstr );
		}
		item->setCheckState( function->f_mode ? Qt::Checked : Qt::Unchecked );
		m_mainWidget->lb_fktliste->sortItems();
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
	if( editParametric->exec() == QDialog::Accepted )
	{
		Ufkt *function_y = editParametric->functionItem();
		Ufkt *function_x = &m_view->parser()->ufkt[m_view->parser()->ixValue(function_y->id - 1)]; //get the x-function
		QListWidgetItem *item;
		if( x_id == -1 ) //a new function
		{
			item = new QListWidgetItem( function_x->fstr+";"+function_y->fstr, m_mainWidget->lb_fktliste );
		}
		else //change a function
		{
			item = m_mainWidget->lb_fktliste->currentItem();
			item->setText( function_x->fstr+";"+function_y->fstr );
		}
		item->setCheckState( function_y->f_mode ? Qt::Checked : Qt::Unchecked );
		m_mainWidget->lb_fktliste->sortItems();
		changed = true;
		updateView();
	}
}

void FktDlg::slotEditPolar( int id )
{
	KEditPolar* editPolar = new KEditPolar( m_view->parser(), this );
	if ( id==-1) editPolar->setCaption(i18n( "New Polar Plot" ));
	editPolar->initDialog( id );
	if( editPolar->exec() == QDialog::Accepted )
	{
		Ufkt *function = editPolar->functionItem();
		QListWidgetItem *item;
		if( id == -1 ) // a new function
		{
			item = new QListWidgetItem( function->fstr, m_mainWidget->lb_fktliste );
		}
		else //change a function
		{
			item = m_mainWidget->lb_fktliste->currentItem();
			item->setText( function->fstr );
		}
		item->setCheckState( function->f_mode ? Qt::Checked : Qt::Unchecked );
		m_mainWidget->lb_fktliste->sortItems();
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
	m_mainWidget->lb_fktliste->clear();

  // adding all yet added functions
	for( QVector<Ufkt>::iterator it = m_view->parser()->ufkt.begin(); it != m_view->parser()->ufkt.end(); ++it)
	{
		if( it->fname.isEmpty() || it->fstr[0] == 'y' ) continue;
		QListWidgetItem *item;
		if( it->fstr[0] == 'x' )
		{
			QString y = it->fstr;
			++it;
			item = new QListWidgetItem( y + ";" + it->fstr, m_mainWidget->lb_fktliste );
		}
		else
			item = new QListWidgetItem( it->fstr, m_mainWidget->lb_fktliste );
		item->setCheckState( it->f_mode ? Qt::Checked : Qt::Unchecked );
	}
	m_mainWidget->lb_fktliste->sortItems();
}

void  FktDlg::slotHelp()
{
	KToolInvocation::invokeHelp( "", "kmplot" );
}

bool FktDlg::isChanged()
{
	return changed;
}

void FktDlg::showEvent ( QShowEvent * )
{
	changed = false;
}

void FktDlg::slotCopyFunction()
{
	if ( m_mainWidget->lb_fktliste->currentItem() == 0)
	{
		m_mainWidget->cmdCopyFunction->setEnabled(false);
		return;
	}
	QListWidgetItem *currentItem = m_mainWidget->lb_fktliste->currentItem();
	QString const fstr = currentItem->text();
	m_view->parser()->sendFunction(getId(fstr));
}

void FktDlg::slotMoveFunction()
{
	if ( m_mainWidget->lb_fktliste->currentItem() == 0)
	{
		m_mainWidget->cmdCopyFunction->setEnabled(false);
		return;
	}
	QListWidgetItem *currentItem = m_mainWidget->lb_fktliste->currentItem();
	QString const fstr = currentItem->text();
	if ( !m_view->parser()->sendFunction(getId( currentItem->text()) ) )
		return;
	slotDelete();
}
