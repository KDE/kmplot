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

#ifndef KEDITPARAMETRIC_H
#define KEDITPARAMETRIC_H

#include "qeditparametric.h"
#include "xparser.h"

class View;
class XParser;

/** @short Dialog window editing a parametric plot and its properties. */
class KEditParametric : public QEditParametric
{
	Q_OBJECT
	public:
		/// @param parser points to the parser instance.
		/// @param parent points to the parent widget.
		/// @param name of this instance.
		KEditParametric( XParser* parser, QWidget* parent = NULL, const char* name = NULL );
		/// Nothing special to do.
		virtual ~KEditParametric() {}
		
		/// Fill the dialog's widgets with the properties of the parser function number id.
		void initDialog( int x_id = -1, int y_id = -1 );
		/// Returns a pointer to the added/inserted y-function, or 0 if the user havn't pressed OK yet
		Ufkt *functionItem();
	
	private:
		/// Clear alls widgets values
		void clearWidgets();
		/// Fill the dialog's widgets with values from the parser
		void setWidgets();
		/// Returns the well formed function equation
		QString xFunction();
		/// Returns the well formed function equation
		QString yFunction();	
		/// extract function \a name and \a expression from a given \a equation 
		void splitEquation( const QString equation, QString &name, QString &expression );
		/// Pointer to the parser instance.
		XParser* m_parser;
		//@{
		/// Current function indices.
		int m_x_id, m_y_id;
		//@}
		
	protected slots:
		/// Overwrites the dialog's accept() method to make sure, that the user's input is valid.
		virtual void accept();
		/// Invokes the HelpCenter.
		void slotHelp();
		void customMaxRange_toggled(bool);
		void customMinRange_toggled(bool);

		Ufkt *m_updatedfunction;
};

#endif
