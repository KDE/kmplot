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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#ifndef KEDITFUNCTION_H
#define KEDITFUNCTION_H

#include "qeditfunction.h"

class XParser;

class KEditFunction : public QEditFunction
{
	Q_OBJECT
	public:
		KEditFunction( XParser* parser, QWidget* parent = NULL, const char* name = NULL );
		virtual ~KEditFunction() {};
		
		void initDialog( int index = -1 );
		const QString functionItem();
	
	private:
		void clearWidgets();
		void setWidgets();
		XParser* m_parser;
		int m_index;
		
	protected slots:
		virtual void accept();
		void slotHelp();
};

#endif
