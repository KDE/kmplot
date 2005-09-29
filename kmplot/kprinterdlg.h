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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/
/** @file kprinterdlg.h 
 * @brief Contains the KPrinterDlg class. */

#ifndef kprintdlg_included
#define kprintdlg_included

// Qt includes
#include <qcheckbox.h>

// KDE includes
#include <kdeprint/kprintdialogpage.h>

/** @short A dialog page for the print dialog.
 *
 * It manages the option to print or not to print the header table with addition plot information.
 * It is linked to the app-kmplot-printtable entry in the kmplotrc.
 */
class KPrinterDlg : public KPrintDialogPage
{
	Q_OBJECT
public:
	/// Getting the common arguments.
	KPrinterDlg( QWidget *parent = 0, const char *name = 0 );

	/// Reimplemented.
	void getOptions( QMap<QString, QString>& opts, bool include_def = false );
	/// Reimplemented.
	void setOptions( const QMap<QString, QString>& opts );
	/// Reimplemented.
	bool isValid( const QString& msg );
	/// The check box for the option.
	QCheckBox *printHeaderTable;
	QCheckBox *transparent_background;
};

#endif //kprinterdlg_included
