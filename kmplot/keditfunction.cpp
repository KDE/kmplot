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
#include <qcheckbox.h>

// KDE includes
#include <klineedit.h>

// local includes
#include "keditfunction.h"
#include "keditfunction.moc"
#include "xparser.h"

KEditFunction::KEditFunction( XParser* parser, QWidget* parent, const char* name ) : 
	QEditFunction( parent, name )
{
	m_parser = parser;
}

void KEditFunction::setType( const FunctionType t )
{
	m_type = t;
	switch( t )
	{
		case Function: 
			this->setCaption( "Edit Function Plot" );
			kLineEditXFunction->hide();
			checkBoxDerivative1->show();
			checkBoxDerivative2->show();
			break;
		case Parametric: 
			this->setCaption( "Edit Parametric Plot" );
			kLineEditXFunction->show();
			checkBoxDerivative1->hide();
			checkBoxDerivative2->hide();
			break;
		case Polar: 
			this->setCaption( "Edit Polar Plot" );
	}
	updateGeometry();
}

void KEditFunction::accept()
{
	int index;
	if( kLineEditXFunction->isVisible() )
	{ 
		index = m_parser->addfkt( kLineEditXFunction->text() );
		if( index == -1 ) 
		{
			m_parser->errmsg();
			this->raise();
			kLineEditXFunction->setFocus();
			kLineEditXFunction->selectAll();
			return;
		}
	}
	if( kLineEditYFunction->isVisible() )
	{ 
		index = m_parser->addfkt( kLineEditYFunction->text() );
		if( index == -1 ) 
		{
			m_parser->errmsg();
			this->raise();
			kLineEditYFunction->setFocus();
			kLineEditYFunction->selectAll();
			return;
		}
	}
	QEditFunction::accept();
}
