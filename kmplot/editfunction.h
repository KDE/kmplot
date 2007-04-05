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

#ifndef KEDITFUNCTION_H
#define KEDITFUNCTION_H

#include <kdialogbase.h>

#include "xparser.h"

class XParser;
class EditFunctionPage;
class EditDerivativesPage;
class EditIntegralPage;

/** @short Dialog window editing a function plot and its properties. */
class EditFunction : public KDialogBase
{
	Q_OBJECT
	public:
		/// @param parser points to the parser instance.
		/// @param parent points to the parent widget.
		/// @param name of this instance.
		EditFunction( XParser* parser, QWidget* parent = NULL, const char* name = NULL );
		/// Nothing special to do.
		virtual ~EditFunction() {}
		/// Fill the dialog's widgets with the properties of the parser function number index.
		void initDialog( int id = -1 );
		/// Returns a pointer to the added/updated function
		Ufkt *functionItem();
	
	protected slots:
		/// Overwrites the dialog's accept() method to make sure, that the user's input is valid.
		virtual void accept();
		/// Invokes the HelpCenter.
		void slotHelp();
		/// Called when the "Change parameter values" is clicked
		void cmdParameter_clicked();
		/// Called when the no_parameter-radiobutton is toggled.
		void noParameter_toggled(bool);
		/// Called when customMinRange is toggled.
		void customMinRange_toggled(bool);
		/// Called when customMaxRange is toggled.
		void customMaxRange_toggled(bool);
	
	private:
		/// Clear alls widgets values
		void clearWidgets();
		/// Fill the dialog's widgets with values from the parser
		void setWidgets();
		/// Check, if the function has 2 arguments (function group)
		bool functionHas2Arguments();
		/// Adding an extra argument
		void fixFunctionArguments(QString &);
		
		/// Pointer to the parser instance
		XParser* m_parser;
		/// Current function index
		int m_id;
		QValueList<ParameterValueItem> m_parameter;
		
		EditFunctionPage* editfunctionpage;
		EditDerivativesPage* editderivativespage;
		EditIntegralPage* editintegralpage;

        Ufkt *m_updatedfunction;
};

#endif
