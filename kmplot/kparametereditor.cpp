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


#include <kdebug.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <ktempfile.h>
#include <kurl.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qvaluelist.h>

#include "kparametereditor.h"

class ParameterValueList;

KParameterEditor::KParameterEditor(XParser *m, QValueList<ParameterValueItem> *l, QWidget *parent, const char *name)
	: QParameterEditor(parent,name, true, Qt::WDestructiveClose), m_parameter(l), m_parser(m)
{
	for (  QValueList<ParameterValueItem>::Iterator it = m_parameter->begin(); it != m_parameter->end(); ++it )
		list->insertItem( (*it).expression );
	list->sort();
	
	connect( cmdNew, SIGNAL( clicked() ), this, SLOT( cmdNew_clicked() ));
	connect( cmdEdit, SIGNAL( clicked() ), this, SLOT( cmdEdit_clicked() ));
	connect( cmdDelete, SIGNAL( clicked() ), this, SLOT( cmdDelete_clicked() ));
	connect( cmdImport, SIGNAL( clicked() ), this, SLOT( cmdImport_clicked() ));
	connect( cmdExport, SIGNAL( clicked() ), this, SLOT( cmdExport_clicked() ));
	connect( cmdClose, SIGNAL( clicked() ), this, SLOT( close() ));
	connect( list, SIGNAL( doubleClicked( QListBoxItem * ) ), this, SLOT( varlist_doubleClicked( QListBoxItem *) ));
	connect( list, SIGNAL( clicked ( QListBoxItem * ) ), this, SLOT( varlist_clicked(QListBoxItem *  ) ));
	
}

KParameterEditor::~KParameterEditor()
{
	m_parameter->clear();
	QString item_text;
	for (int i = 0; (uint)i <= list->count();i++)
	{
		item_text = list->text(i);
		if ( !item_text.isEmpty() )
			m_parameter->append( ParameterValueItem(item_text, m_parser->eval( item_text)) );
	}
}

void KParameterEditor::cmdNew_clicked()
{
	QString result="";
	while (1)
	{
		bool ok;
		result = KInputDialog::getText( i18n("Parameter Value"), i18n( "Enter a new parameter value:" ), result, &ok );
		if ( !ok)
			return;
		m_parser->eval( result );
		if ( m_parser->parserError(false) != 0 )
		{
			m_parser->parserError();
			continue;
		}
		if ( checkTwoOfIt(result) )
		{
			KMessageBox::error(0,i18n("The value %1 already exists and will therefore not be added.").arg(result));
			continue;
		}
		list->insertItem(result);
		list->sort();
		break;
	}
}

void KParameterEditor::cmdEdit_clicked()
{
	QString result=list->currentText();
	while (1)
	{
		bool ok;
		result = KInputDialog::getText( i18n("Parameter Value"), i18n( "Enter a new parameter value:" ), result, &ok );
		if ( !ok)
			return;
		m_parser->eval(result);
		if ( m_parser->parserError(false) != 0)
		{
			m_parser->parserError();
			continue;
		}
		if ( checkTwoOfIt(result) )
		{
			if( result != list->currentText() )
				KMessageBox::error(0,i18n("The value %1 already exists.").arg(result));
			continue;
		}
		list->removeItem( list->currentItem());
		list->insertItem(result);
		list->sort();
		break;
	}
}

void KParameterEditor::cmdDelete_clicked()
{
	list->removeItem( list->currentItem());
	list->sort();
}

void KParameterEditor::cmdImport_clicked()
{
	KURL url = KFileDialog::getOpenURL( QString::null,i18n("*.txt|Plain Text File "));
	if ( url.isEmpty() )
		return;
        
        if (!KIO::NetAccess::exists(url,true,this) )
        {
                KMessageBox::error(0,i18n("The file does not exist."));
                return;
        }
        
	bool verbose = false;
        QFile file;
        QString tmpfile;
        if ( !url.isLocalFile() )
        {
                if ( !KIO::NetAccess::download(url, tmpfile, this) )
                {
                        KMessageBox::error(0,i18n("An error appeared when opening this file"));
                        return;
                }
                file.setName(tmpfile);
        }
        else
                file.setName(url.prettyURL(0,KURL::StripFileProtocol) );
	
	if ( file.open(IO_ReadOnly) )
	{
		QTextStream stream(&file);
		QString line;
		for( int i=1; !stream.atEnd();i++ )
		{
			line = stream.readLine();
			if (line.isEmpty())
				continue;
			m_parser->eval( line );
			if ( m_parser->parserError(false) == 0)
			{
				if ( !checkTwoOfIt(line) )
				{
					list->insertItem(line);
					list->sort();
				}
			}
			else if ( !verbose)
			{
				if ( KMessageBox::warningContinueCancel(this,i18n("Line %1 is not a valid parameter value and will therefore not be included. Do you want to continue?").arg(i) ) == KMessageBox::Cancel)
				{
					file.close();
                                        KIO::NetAccess::removeTempFile( tmpfile );
					return;
				}
				else if (KMessageBox::warningYesNo(this,i18n("Would you like to be informed about other lines that cannot be read?"), QString::null, i18n("Get Informed"), i18n("Ignore Information") ) == KMessageBox::No)
					verbose = true;
			}
		}
		file.close();
	}
	else
		KMessageBox::error(0,i18n("An error appeared when opening this file"));
        
        if ( !url.isLocalFile() )
                KIO::NetAccess::removeTempFile( tmpfile );
}

void KParameterEditor::cmdExport_clicked()
{
        if ( !list->count() )
                return;
        KURL url = KFileDialog::getSaveURL( QString::null,i18n("*.txt|Plain Text File "));
        if ( url.isEmpty() )
                return;

        if( !KIO::NetAccess::exists( url,false,this ) || KMessageBox::warningContinueCancel( this, i18n( "A file named \"%1\" already exists. Are you sure you want to continue and overwrite this file?" ).arg( url.url()), i18n( "Overwrite File?" ), KGuiItem( i18n( "&Overwrite" ) ) ) == KMessageBox::Continue )
        {
                QString tmpfile;
                QFile file;
                if ( !url.isLocalFile() )
                {
                        KTempFile tmpfile;
                        file.setName(tmpfile.name() );
                        
                        if (file.open( IO_WriteOnly ) )
                        {
                                QTextStream stream(&file);
                                QListBoxItem *it = list->firstItem();
                                while ( 1 )
                                {
                                        stream << it->text();
                                        it = it->next();
                                        if (it)
                                                stream << endl; //only write a new line if there are more text
                                        else
                                                break;
                                }
                                file.close();
                        }
                        else
                                KMessageBox::error(0,i18n("An error appeared when saving this file"));
                        
                        if ( !KIO::NetAccess::upload(tmpfile.name(),url, this) )
                        {
                                KMessageBox::error(0,i18n("An error appeared when saving this file"));
                                tmpfile.unlink();
                                return;
                        }
                        tmpfile.unlink();
                }
                else
                {
                        file.setName(url.prettyURL(0,KURL::StripFileProtocol));
                        if (file.open( IO_WriteOnly ) )
                        {
                                QTextStream stream(&file);
                                QListBoxItem *it = list->firstItem();
                                while ( 1 )
                                {
                                        stream << it->text();
                                        it = it->next();
                                        if (it)
                                                stream << endl; //only write a new line if there are more text
                                        else
                                                break;
                                }
                                file.close();
                        }
                        else
                                KMessageBox::error(0,i18n("An error appeared when saving this file"));
                }
        }


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

bool KParameterEditor::checkTwoOfIt(const QString & text)
{
	if ( list->findItem(text,Qt::ExactMatch) == 0)
		return false;
	else
		return true;
}

#include "kparametereditor.moc"
