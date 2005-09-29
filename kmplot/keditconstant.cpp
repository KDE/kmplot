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

KEditConstant::KEditConstant(XParser *p, char &c, QString &v, QWidget *parent, const char *name)
	: QEditConstant(parent, name,Qt::WDestructiveClose), constant(c), value(v), m_parser(p)
{
	if ( constant != '0' )
	{
		txtConstant->setEnabled(false);
		txtConstant->setText(QChar(constant));
		txtValue->setText(value);
		txtValue->setFocus();
		txtValue->selectAll();
	}
	connect( cmdCancel, SIGNAL( clicked() ), this, SLOT( deleteLater() ));
	connect( cmdOK, SIGNAL( clicked() ), this, SLOT( cmdOK_clicked() ));
}


KEditConstant::~KEditConstant()
{
}

void KEditConstant::cmdOK_clicked()
{
	constant = txtConstant->text().at(0).latin1();
	value = txtValue->text();
	if ( constant<'A' || constant>'Z')
	{
		KMessageBox::error(this, i18n("Please insert a valid constant name between A and Z."));
		txtConstant->setFocus();
		txtConstant->selectAll();
		return;
	}
	
	if ( txtConstant->isEnabled() ) //creating, not edit a constant
	{
		bool found= false;
		QValueVector<Constant>::iterator it;
		for(it = m_parser->constant.begin(); it!= m_parser->constant.end() && !found;++it)
		{
			if ( it->constant == constant)
					found = true;
		}
		if (found)
		{
			KMessageBox::error(this, i18n("The constant already exists."));
			return;
		}	
	}
	(double) m_parser->eval(value);
	if ( m_parser->parserError() )
	{
		txtValue->setFocus();
		txtValue->selectAll();
		return;
	}
	
	emit finished();
	QDialog::accept();
}

void KEditConstant::txtVariable_lostFocus()
{
    txtConstant->setText( txtConstant->text().upper() );
}

#include "keditconstant.moc"

