/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ller
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

/** @file AttrDlg.h
 * @brief contains the AttrDlg class, a dialog for the attributes of the functions.
 */
#ifndef AttrDlg_included
#define AttrDlg_included

// Qt includes
#include <qcheckbox.h>
#include <qlineedit.h>

// KDE includes
#include <kapplication.h>
#include <kcolordialog.h>
#include <klocale.h>

// local includes
#include "AttrDlgData.h"
#include "misc.h"

/**
 * This class provides the input of attributes to the function's graph.
 * The GUI is produced with Qt's Designer.
 */
class AttrDlg : public AttrDlgData
{
	Q_OBJECT

public:
	/**
	 * The constructor gets the common parameters.
	 * The modal flag is set to true by default.
	 */
	AttrDlg( const int ix_, QWidget* parent = NULL, const char* name = NULL );
	/** The destructor does nothing special */
	virtual ~AttrDlg();

	/**
	 * The index of the current function.
	 * @see FktDlg::onattr()
	 * */
	int ix;

private:

	/** Temporary instance of the chosen color. */
	//QColor farbe;


protected slots:
	/**
	 * Connected to the Clicked signal of the "Ok" button.
	 * The settings are applied.
	 * If "save as default" is checked, the global config file is changed.
	 * @see kc
	 */
	void onok();
	/**
	 * Connected to the Clicked signal of the "Cancel" button.
	 * Closes the windows without any changes.
	 */
	void oncancel();
	/**
	 * Connected to the Clicked signal of the "Color..." Button.
	 * Calls a sub-dialog for the color setting.
	 * @todo Port to KColorButton
	 */
	//void onfarbe();

	/** Here some additional initialization is done. */
	// virtual	void show();

};

#endif // AttrDlg_included
