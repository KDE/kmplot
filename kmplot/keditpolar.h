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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#ifndef KEDITPOLAR_H
#define KEDITPOLAR_H

#include "qeditpolar.h"
#include "xparser.h"

class View;
class XParser;

/** @short Dialog window editing a polar plot and its properties. */
class KEditPolar : public QEditPolar
{
	Q_OBJECT
	public:
		/// @param parser points to the parser instance.
		/// @param parent points to the parent widget.
		/// @param name of this instance.
		KEditPolar( XParser* parser, QWidget* parent = NULL, const char* name = NULL );
		/// Nothing special to do.
		virtual ~KEditPolar() {}
		
		///Fill the dialog widgets with the properties of the parser function number id.
		void initDialog( int id = -1 );
		/// Returns a pointer to the added/updated function
		Ufkt * functionItem();
	
	private:
		/// Clear alls widgets values.
		void clearWidgets();
		/// Fill the dialog's widgets with values from the parser.
		void setWidgets();
		/// Pointer to the parser instance.
		XParser* m_parser;
		/// Current function id.
		int m_id;
		
	protected slots:
		/// Overwrites the dialog's accept() method to make sure, that the user's input is valid.
		virtual void accept();
		/// Invokes the helpCenter.
		void slotHelp();
		void customMinRange_toggled(bool);
		void customMaxRange_toggled(bool);
		
		Ufkt *m_updatedfunction;
};

#endif
