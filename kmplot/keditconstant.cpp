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
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qpushbutton.h>


#include "keditconstant.h"

KEditConstant::KEditConstant(XParser *p, char &c, QString &v, QWidget *parent)
	: KDialog( parent, i18n("Edit Constant"), Ok|Cancel ),
	  constant(c), value(v), m_parser(p)
{
	m_mainWidget = new QEditConstant( this );
	setMainWidget( m_mainWidget );
	
	if ( constant != '0' )
	{
		m_mainWidget->txtConstant->setEnabled(false);
		m_mainWidget->txtConstant->setText(QString(constant));
		m_mainWidget->txtValue->setText(value);
		m_mainWidget->txtValue->setFocus();
		m_mainWidget->txtValue->selectAll();
	}
	
	connect( m_mainWidget->txtConstant, SIGNAL( textChanged( const QString & ) ), this, SLOT( nameChanged( const QString & ) ) );
}


KEditConstant::~KEditConstant()
{
}

void KEditConstant::accept()
{
	constant = m_mainWidget->txtConstant->text().at(0).latin1();
	value = m_mainWidget->txtValue->text();
	if ( constant<'A' || constant>'Z')
	{
		KMessageBox::sorry(this, i18n("Please insert a valid constant name between A and Z."));
		m_mainWidget->txtConstant->setFocus();
		m_mainWidget->txtConstant->selectAll();
		return;
	}
	
	if ( m_mainWidget->txtConstant->isEnabled() ) //creating, not edit a constant
	{
		bool found= false;
		QVector<Constant>::iterator it;
		for(it = m_parser->constant.begin(); it!= m_parser->constant.end() && !found;++it)
		{
			if ( it->constant == constant)
					found = true;
		}
		if (found)
		{
			KMessageBox::sorry(this, i18n("The constant already exists."));
			return;
		}	
	}
	(double) m_parser->eval(value);
	if ( m_parser->parserError() )
	{
		m_mainWidget->txtValue->setFocus();
		m_mainWidget->txtValue->selectAll();
		return;
	}
	
	emit finished();
	QDialog::accept();
}


void KEditConstant::nameChanged( const QString & newName )
{
	m_mainWidget->txtConstant->setText( newName.toUpper() );
}

#include "keditconstant.moc"

