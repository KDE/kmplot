//
// C++ Interface: kconstanteditor
//
// Description: 
//
//
// Author: Fredrik Edemar <f_edemar@linux.se>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kapplication.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include <qdom.h>

#include "kmplotio.h"
#include "kconstanteditor.h"


KConstantEditor::KConstantEditor(QWidget *parent, const char *name)
				: QConstantEditor(parent, name)
{
	
}
KConstantEditor::KConstantEditor(View *v, QWidget *parent, const char *name)
				: QConstantEditor(parent,name)
{
	m_view = v;
	QString str_value;
	QValueVector<Constant>::iterator it;
	for(it = m_view->parser()->constant.begin(); it!= m_view->parser()->constant.end() ;it++)
	{
		str_value.setNum(it->value);
		(void) new QListViewItem(varlist, QChar(it->constant), str_value);
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
	dlg->txtConstant->setEnabled(false);
	dlg->show();
}

void KConstantEditor::cmdDelete_clicked()
{
	if ( !varlist->currentItem() )
		return;
	int index;
	QString fname, fstr, str;
	bool stop = false;
	constant = varlist->currentItem()->text(0).at(0).latin1();
	value = varlist->currentItem()->text(1);
	
	
	
	for ( index = 0; index < m_view->parser()->ufanz  && !stop; ++index )
	{
		if ( m_view->parser()->getfkt( index, fname, fstr ) == -1 ) continue;
		str =  m_view->parser()->fktext[ index ].extstr ;
		for (int i=str.find(')'); i<str.length() && !stop;i++)
			if ( str.at(i) == constant )
				stop = true;
	}
	if (stop)
	{
		KMessageBox::error(this, i18n("A function uses this constant and therefore it can't be removed"));
		return;
	}
	
	QValueVector<Constant>::iterator it;
	for(it = m_view->parser()->constant.begin(); it!= m_view->parser()->constant.end() && !stop;it++)
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
			stop = true;
		}
	}
	if (!stop)
	{
		KMessageBox::error(this, i18n("The item couln't be found"));
		return;
	}
	
	QListViewItem *item = varlist->findItem(QChar(constant), 0);
	if (item!=0)
		varlist->takeItem(item);
}

void KConstantEditor::varlist_clicked( QListViewItem * item )
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
		QValueVector<Constant>::iterator it;
		for(it = m_view->parser()->constant.begin(); it!= m_view->parser()->constant.end() && !found;it++)
		{
			if ( it->constant == i || i == constant)
			{
				found = true;
			}
		}
		if (!found)
			list.append(QChar(i));
	}
	QStringList result = KInputDialog::getItemList("Kmplot",i18n("Choose a name for the constant:"),list, QStringList(), false, &found);
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
	(void) new QListViewItem(varlist, QChar( constant  ), value);
	varlist->sort();
}

void KConstantEditor::editConstantSlot()
{
	double dvalue = m_view->parser()->eval(value);
	bool found = false;
	QValueVector<Constant>::iterator it;
	for(it = m_view->parser()->constant.begin(); it!= m_view->parser()->constant.end() && !found;it++)
	{
		if ( it->constant == constant)
		{
			it->value = dvalue;
			found = true;
		}
	}
	if (!found)
	{
		KMessageBox::error(this, i18n("The item couln't be found"));
		return;
	}

	QListViewItem *item = varlist->findItem(QChar(constant), 0);
	if (item!=0)
		item->setText(1,value);
	
	QString tmpName = locate("tmp", "") + "kmplot-" + KApplication::kApplication()->sessionId();
	KmPlotIO::save( m_view->parser(), tmpName );
	m_view->init();
	KmPlotIO::load( m_view->parser(), tmpName );
	QFile::remove( tmpName );
	
	m_view->drawPlot();
}

void KConstantEditor::varlist_doubleClicked( QListViewItem * )
{
	cmdEdit_clicked();
}

#include "kconstanteditor.moc"
