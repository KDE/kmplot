/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar
*                     f_edemar@linux.se
*               2006  David Saxton <david@bluehaze.org>
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
#include <QList>
#include <QListWidget>

#include "kparametereditor.h"

#include <assert.h>

class ParameterValueList;

KParameterEditor::KParameterEditor(XParser *m, QList<Value> *l, QWidget *parent )
	: KDialog( parent, i18n("Parameter Editor"), Ok|Cancel ),
	  m_parameter(l),
	  m_parser(m)
{
	m_mainWidget = new QParameterEditor( this );
	setMainWidget( m_mainWidget );
	
	foreach ( Value v, *m_parameter )
		m_mainWidget->list->addItem( v.expression() );
	m_mainWidget->list->sortItems();
	
	connect( m_mainWidget->cmdNew, SIGNAL( clicked() ), this, SLOT( cmdNew_clicked() ));
	connect( m_mainWidget->cmdDelete, SIGNAL( clicked() ), this, SLOT( cmdDelete_clicked() ));
	connect( m_mainWidget->cmdImport, SIGNAL( clicked() ), this, SLOT( cmdImport_clicked() ));
	connect( m_mainWidget->cmdExport, SIGNAL( clicked() ), this, SLOT( cmdExport_clicked() ));
	connect( m_mainWidget->list, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem * )), this, SLOT(selectedConstantChanged( QListWidgetItem * )) );
	
	connect( m_mainWidget->value, SIGNAL( textEdited( const QString & ) ), this, SLOT( saveCurrentValue() ) );
	connect( m_mainWidget->value, SIGNAL( textChanged( const QString & ) ), this, SLOT( checkValueValid() ) );
	
	checkValueValid();
	
	m_mainWidget->value->setFocus();
}

KParameterEditor::~KParameterEditor()
{
}


void KParameterEditor::accept()
{
	kDebug() << "saving\n";
	m_parameter->clear();
	QString item_text;
	for ( int i = 0; i < m_mainWidget->list->count(); i++ )
	{
		item_text = m_mainWidget->list->item(i)->text();
		if ( !item_text.isEmpty() )
		{
			Value value;
			if ( value.updateExpression( item_text ) )
				m_parameter->append( value );
		}
	}
	
	KDialog::accept();
}


void KParameterEditor::cmdNew_clicked()
{
	QListWidgetItem * item = new QListWidgetItem( m_mainWidget->list );
	m_mainWidget->list->setCurrentItem( item );
	m_mainWidget->value->setFocus();
}


void KParameterEditor::selectedConstantChanged( QListWidgetItem * current )
{
	m_mainWidget->cmdDelete->setEnabled( current != 0 );
	m_mainWidget->value->setText( current ? current->text() : QString::null );
}


void KParameterEditor::cmdDelete_clicked()
{
	QListWidgetItem * item = m_mainWidget->list->currentItem();
	if ( !item )
		return;
	
	m_mainWidget->value->clear();
	m_mainWidget->list->takeItem( m_mainWidget->list->currentRow() );
	delete item;
	
	m_mainWidget->cmdDelete->setEnabled( m_mainWidget->list->currentItem() != 0 );
	m_mainWidget->list->sortItems();
}


void KParameterEditor::saveCurrentValue()
{
	QListWidgetItem * current = m_mainWidget->list->currentItem();
	if ( !current )
		current = new QListWidgetItem( m_mainWidget->list );
	
	current->setText( m_mainWidget->value->text() );
	m_mainWidget->list->setCurrentItem( current );
}


bool KParameterEditor::checkValueValid()
{
	QString valueText = m_mainWidget->value->text();
	(double) m_parser->eval( valueText );
	bool valid = (m_parser->parserError( false ) == 0);
	m_mainWidget->valueInvalidLabel->setVisible( !valueText.isEmpty() && !valid );
	return valid;
}


void KParameterEditor::cmdImport_clicked()
{
	KUrl url = KFileDialog::getOpenURL( QString(),i18n("*.txt|Plain Text File "));
	if ( url.isEmpty() )
		return;
        
        if (!KIO::NetAccess::exists(url,true,this) )
        {
			KMessageBox::sorry(0,i18n("The file does not exist."));
                return;
        }
        
	bool verbose = false;
        QFile file;
        QString tmpfile;
        if ( !url.isLocalFile() )
        {
                if ( !KIO::NetAccess::download(url, tmpfile, this) )
                {
					KMessageBox::sorry(0,i18n("An error appeared when opening this file"));
                        return;
                }
                file.setFileName(tmpfile);
        }
        else
                file.setFileName(url.path() );
	
	if ( file.open(QIODevice::ReadOnly) )
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
					m_mainWidget->list->addItem(line);
					m_mainWidget->list->sortItems();
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
				else if (KMessageBox::warningYesNo(this,i18n("Would you like to be informed about other lines that cannot be read?"), QString(), i18n("Get Informed"), i18n("Ignore Information") ) == KMessageBox::No)
					verbose = true;
			}
		}
		file.close();
	}
	else
		KMessageBox::sorry(0,i18n("An error appeared when opening this file"));
        
        if ( !url.isLocalFile() )
                KIO::NetAccess::removeTempFile( tmpfile );
}


void KParameterEditor::cmdExport_clicked()
{
	if ( !m_mainWidget->list->count() )
                return;
        KUrl url = KFileDialog::getSaveURL( QString(),i18n("*.txt|Plain Text File "));
        if ( url.isEmpty() )
                return;

        if( !KIO::NetAccess::exists( url,false,this ) || KMessageBox::warningContinueCancel( this, i18n( "A file named \"%1\" already exists. Are you sure you want to continue and overwrite this file?" ).arg( url.url()), i18n( "Overwrite File?" ), KGuiItem( i18n( "&Overwrite" ) ) ) == KMessageBox::Continue )
        {
                QString tmpfile;
                QFile file;
                if ( !url.isLocalFile() )
                {
                        KTempFile tmpfile;
                        file.setFileName(tmpfile.name() );
                        
                        if (file.open( QIODevice::WriteOnly ) )
                        {
							QTextStream stream(&file);
							for ( int i = 0; i < m_mainWidget->list->count(); i++ )
							{
								QListWidgetItem * it = m_mainWidget->list->item( i );
								stream << it->text();
								if ( i < m_mainWidget->list->count()-1 )
									stream << endl; //only write a new line if there are more text
							}
							file.close();
                        }
						else
							KMessageBox::sorry(0,i18n("An error appeared when saving this file"));
                        
                        if ( !KIO::NetAccess::upload(tmpfile.name(),url, this) )
                        {
							KMessageBox::sorry(0,i18n("An error appeared when saving this file"));
                                tmpfile.unlink();
                                return;
                        }
                        tmpfile.unlink();
                }
                else
                {
					kDebug() << "url.path()="<<url.path()<<endl;
                        file.setFileName(url.path());
                        if (file.open( QIODevice::WriteOnly ) )
                        {
							QTextStream stream(&file);
							for ( int i = 0; i < m_mainWidget->list->count(); i++ )
							{
								QListWidgetItem * it = m_mainWidget->list->item( i );
								stream << it->text();
								if ( i < m_mainWidget->list->count()-1 )
									stream << endl; //only write a new line if there are more text
							}
							file.close();
                        }
                        else
							KMessageBox::sorry(0,i18n("An error appeared when saving this file"));
                }
        }


}


bool KParameterEditor::checkTwoOfIt(const QString & text)
{
	return !m_mainWidget->list->findItems(text,Qt::MatchExactly).isEmpty();
}

#include "kparametereditor.moc"
