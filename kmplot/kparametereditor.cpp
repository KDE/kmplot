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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/


#include <kdebug.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <qfile.h>
#include <qtextstream.h>

#include "kparametereditor.h"

KParameterEditor::KParameterEditor(QWidget *parent, const char *name)
				: QParameterEditor(parent, name)
{
	
}
KParameterEditor::KParameterEditor(XParser *m, QStringList *l, QWidget *parent, const char *name)
	: QParameterEditor(parent,name, true, Qt::WDestructiveClose), m_parameter(l), m_parser(m)
{
  	for ( QStringList::Iterator it = m_parameter->begin(); it != m_parameter->end(); ++it )
		list->insertItem(*it);
	list->sort();
	
	connect( cmdNew, SIGNAL( clicked() ), this, SLOT( cmdNew_clicked() ));
	connect( cmdEdit, SIGNAL( clicked() ), this, SLOT( cmdEdit_clicked() ));
	connect( cmdDelete, SIGNAL( clicked() ), this, SLOT( cmdDelete_clicked() ));
	connect( cmdImport, SIGNAL( clicked() ), this, SLOT( cmdImport_clicked() ));
	connect( cmdClose, SIGNAL( clicked() ), this, SLOT( close() ));
	connect( list, SIGNAL( doubleClicked( QListBoxItem * ) ), this, SLOT( varlist_doubleClicked( QListBoxItem *) ));
	connect( list, SIGNAL( clicked ( QListBoxItem * ) ), this, SLOT( varlist_clicked(QListBoxItem *  ) ));
	
}

KParameterEditor::~KParameterEditor()
{
	m_parameter->clear();
	for (int i = 0; (uint)i <= list->count();i++)
	{
		if ( !list->text(i).isEmpty() )
			m_parameter->append(list->text(i));
	}
}

void KParameterEditor::cmdNew_clicked()
{
	bool ok;
	QString result = KInputDialog::getText( "Kmplot", i18n( "Enter a new parameter value:" ), QString::null, &ok );
	if ( !ok)
		return;
	m_parser->eval( result );
	if ( m_parser->err != 0 )
	{
		m_parser->errmsg();
		cmdNew_clicked();
		return;
	}
	list->insertItem(result);
	list->sort();
}

void KParameterEditor::cmdEdit_clicked()
{
	bool ok;
	QString result = KInputDialog::getText( "Kmplot", i18n( "Enter a new parameter value:" ), QString::null, &ok );
	if ( !ok)
		return;
	m_parser->eval(result);
	if ( m_parser->err != 0 )
	{
		m_parser->errmsg();
		//KMessageBox::error( 0, i18n("Please insert a valid parameter value" ) );
		cmdNew_clicked();
		return;
	}
	list->removeItem( list->currentItem());
	list->insertItem(result);
	list->sort();
}

void KParameterEditor::cmdDelete_clicked()
{
	list->removeItem( list->currentItem());
	list->sort();
}

void KParameterEditor::cmdImport_clicked()
{
	QString filename = KFileDialog::getOpenFileName( QString::null,i18n("*.txt|Plain Text File "));
	if ( filename.isEmpty() )
		return;
	
	QFile file(filename);
	if ( file.open(IO_ReadOnly) )
	{
		QTextStream stream(&file);
		QString line;
		for( int i=1; !stream.atEnd();i++ )
		{
			line = stream.readLine();
			m_parser->eval( line );
			if ( m_parser->err == 0 )
			{
				list->insertItem(line);
				list->sort();
			}
			else
				if ( KMessageBox::warningYesNo(this,i18n("Line %1 is not a valid parameter value and will therefore not be included. Do you want to continue?").arg(i) ) == KMessageBox::No)
				{
					file.close();
					return;
				}
		}
		file.close();
	}
	else
		KMessageBox::error(0,i18n("An error appeared when opening this file"));
}

void KParameterEditor::varlist_clicked( QListBoxItem * item )
{
	if (item)
	{
		cmdEdit->setEnabled(true);
		cmdDelete->setEnabled(true);
	}
	else
	{
		cmdEdit->setEnabled(false);
		cmdDelete->setEnabled(false);		
	}
}


void KParameterEditor::varlist_doubleClicked( QListBoxItem * )
{
	cmdEdit_clicked();
}

#include "kparametereditor.moc"
