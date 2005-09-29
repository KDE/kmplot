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

/** @file FktDlg.h
 * @brief Contains the FktDlg class. Entering and editing functions equation and attributes.
 */

#ifndef FktDlg_included
#define FktDlg_included

// locale includes
#include "FktDlgData.h"
#include "View.h"

class XParser;

/** @short This widget class handles the users function input. 
 *
 * The list of stored plots is shown. The entries can be edited and/or removed. 
 * New plots can be entered calling special dialog windows.
 * @see KEditFunction, KEditParametric, KEditPolar
 */
class FktDlg : public FktDlgData
{
	Q_OBJECT

public:
	/** 
	 * The constructor gets the current parser instance
	 * @param parent Parent widget.
	 * @param view points to the current view instance.
	 */
	FktDlg( QWidget* parent, View* view );
	/// Empty destructor.
	virtual ~FktDlg();
	/// Fill the widgets with plots contained in the parser instance.
	void getPlots();
	/// Returns true if a function was changed
	bool isChanged();

public slots:
	/// Pressed on the "Copy Function.." button
	void slotCopyFunction();
	/// Pressed on the "Move Function.." button
	void slotMoveFunction();
protected slots:
	/// Delete selected plot
	void slotDelete();
	/// Edit selected Plot
	void slotEdit();
	/// Enables/disables actions if the list has a/no selection.
	void slotHasSelection();
	void lb_fktliste_doubleClicked(QListViewItem *, const QPoint &, int);
	void lb_fktliste_clicked(QListViewItem * item);
	void lb_fktliste_spacePressed(QListViewItem * item);
	
	/// Edit a function plot.
	/// @param id Id of the function plot to edit
	/// @see KEditFunction
	void slotEditFunction( int id = -1 );
	/// Edit a parametric plot.
	/// @param x_id Function index to edit
	/// @param y_id Function index to edit
	/// @see KEditParametric
	void slotEditParametric( int x_id = -1, int y_id = -1 );
	/// Edit a polar plot.
	/// @param id Function index of the parser instance.
	/// @see KEditPolar
	void slotEditPolar( int id = -1 );
	/// Edit a new function plot.
	void slotNewFunction();
	/// Edit a new parametric plot.
	void slotNewParametric();
	/// Edit a new polar plot.
	void slotNewPolar();
	/// Invoke Help
	void slotHelp();
	
private:
	/// Looks up the id of \a f_str in the parser instance.
	int getId( const QString &f_str );
	/// Looks up the indices of the parametric pair of function.
	int getParamId( const QString &f_str );
	/// Update the view of the main window.
	void updateView();
	/// Called when the dialog is showed
	void showEvent ( QShowEvent * );
	// /// Send a function to an other instance of Kmplot. Returns true if it success, otherwise false
	// bool sendFunction();
	
	/// Ponts to the parser instance.
	View* m_view;
	/// indicates if a function is changed/added/removed
	bool changed;
};

#endif // FktDlg_included
