/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar
*                     f_edemar@linux.se
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

#include <kapplication.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <q3valuevector.h>
#include <qdom.h>

#include "kmplotio.h"
#include "kconstanteditor.h"


KConstantEditor::KConstantEditor(View *v, QWidget *parent, const char *name)
	: QConstantEditor(parent,name), m_view(v)
{
	QString str_value;
	Q3ValueVector<Constant>::iterator it;
	for(it = m_view->parser()->constant.begin(); it!= m_view->parser()->constant.end() ;++it)
	{
		str_value.setNum(it->value);
		(void) new Q3ListViewItem(varlist, QString(it->constant), str_value);
	}

}

KConstantEditor::~KConstantEditor()
{
}

void KConstantEditor::cmdNew_clicked()
{
	constant = '0';
	KEditConstant *dlg = new KEditConstant(m_view->parser(), constant, value);
	connect( dlg, SIGNAL( finished() ), this,SLOT(newConstantSlot() ) ); 
	dlg->show();
}

void KConstantEditor::cmdEdit_clicked()
{
	if ( !varlist->currentItem() )
		return;
	constant = varlist->currentItem()->text(0).at(0).latin1();
	value = varlist->currentItem()->text(1);
		
	KEditConstant *dlg = new KEditConstant(m_view->parser(), constant, value);
	connect( dlg, SIGNAL( finished() ), this,SLOT(editConstantSlot() ) );
	dlg->show();
}

void KConstantEditor::cmdDelete_clicked()
{
	if ( !varlist->currentItem() )
		return;

	constant = varlist->currentItem()->text(0).at(0).latin1();
	value = varlist->currentItem()->text(1);
	QString str;
        
        for( Q3ValueVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
	{
		str =  it->fstr;
		for (int i=str.find(')'); i<str.length();i++)
			if ( str.at(i) == constant )
                        {
			     KMessageBox::error(this, i18n("A function uses this constant; therefore, it cannot be removed."));
                             return;
                        }
	}
	Q3ValueVector<Constant>::iterator it;
	for(it = m_view->parser()->constant.begin(); it!= m_view->parser()->constant.end(); ++it)
	{
		if ( it->constant == constant)
		{
			if (  it++ == m_view->parser()->constant.end())
				m_view->parser()->constant.pop_back();
			else
			{
				it--;
				m_view->parser()->constant.erase(it++);
			}
			KMessageBox::error(this, i18n("The item could not be found."));
                        return;
		}
	}
	
	delete varlist->findItem(QString(constant), 0); //removes the item from the constant list
}

void KConstantEditor::varlist_clicked( Q3ListViewItem * item )
{
	if (item)
	{
		cmdEdit->setEnabled(true);
		cmdDelete->setEnabled(true);
		cmdDuplicate->setEnabled(true);
	}
	else
	{
		cmdEdit->setEnabled(false);
		cmdDelete->setEnabled(false);
		cmdDuplicate->setEnabled(false);		
	}
}

void KConstantEditor::cmdDuplicate_clicked()
{
	if (!varlist->currentItem())
		return;
	constant = varlist->currentItem()->text(0).at(0).latin1();
	value = varlist->currentItem()->text(1);
	
	QStringList list;
	bool found;
	for (char i='A'; i<'Z';i++)
	{
		found = false;
		Q3ValueVector<Constant>::iterator it;
		for(it = m_view->parser()->constant.begin(); it!= m_view->parser()->constant.end() && !found;++it)
		{
			if ( it->constant == i || i == constant)
			{
				found = true;
			}
		}
		if (!found)
			list.append(QString(i));
	}
	QStringList result = KInputDialog::getItemList(i18n("Choose Name"),i18n("Choose a name for the constant:"),list, QStringList(), false, &found);
	if (found)
	{
		constant = (*result.begin()).at(0).latin1();
		emit newConstantSlot();
	}
	
}

void KConstantEditor::newConstantSlot()
{
	double dvalue = m_view->parser()->eval(value);
	m_view->parser()->constant.append( Constant(constant, dvalue) );
	(void) new Q3ListViewItem(varlist, QString( constant ), value);
	varlist->sort();
}

void KConstantEditor::editConstantSlot()
{
	double dvalue = m_view->parser()->eval(value);
	bool found = false;
	Q3ValueVector<Constant>::iterator it;
	for(it = m_view->parser()->constant.begin(); it!= m_view->parser()->constant.end() && !found;++it)
	{
		if ( it->constant == constant)
		{
			it->value = dvalue;
			found = true;
		}
	}
	if (!found)
	{
		KMessageBox::error(this, i18n("The item could not be found."));
		return;
	}

	Q3ListViewItem *item = varlist->findItem(QString(constant), 0);
	if (item!=0)
		item->setText(1,value);
	

        int index = 0;
        for( Q3ValueVector<Ufkt>::iterator it =  m_view->parser()->ufkt.begin(); it !=  m_view->parser()->ufkt.end(); ++it)
	{
		if( it->fstr.contains(constant)!=0 )
			m_view->parser()->reparse(index); //reparsing the function
                ++index;
	}
	
	m_view->drawPlot();
}

void KConstantEditor::varlist_doubleClicked( Q3ListViewItem * )
{
	cmdEdit_clicked();
}

#include "kconstanteditor.moc"
