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

/** @file BezWnd.h
 * @brief Showing predefined function names and constants (BezWnd).
 */

#ifndef BezWnd_included
#define BezWnd_included

#include "BezWndData.h"

class MainDlg;

/** @short Dialog Window listing all predefined mathematical functions and constants.
 *
 * This Widget shows a list of predefined functions and constants of the implemented math parser.
 * Its connected to a KToggleAction verw_bezeichnungen of the main widget.
 * @see hideEvent, MainDlg::view_bezeichnungen
 */
class BezWnd : public BezWndData
{
	Q_OBJECT

	public:
		/** Common constructor. 
		* It only sets the main_dlg to parent.
		*/
		BezWnd( QWidget* parent = NULL, const char* name = NULL );
		/** Empty destructor. */
		virtual ~BezWnd();
	
	protected slots:
		/** Reimplemented. 
		* Sets the KToggleAction view_bezeichnungen to false.
		* @see MainDlg::slotNames()
		*/
		void hideEvent( QHideEvent * );
	
	private:
		/** A Pointer to the central class instance 
		 * @see MainDlg */
		MainDlg *main_dlg;
};
#endif // BezWnd_included
