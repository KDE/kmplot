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

/** @file misc.h
 * @brief This file contains all global variables and functions. 
 */

#ifndef misc_included
#define misc_included

// Qt includes

// KDE includes
#include <kapplication.h>

// local includes
#include "xparser.h"
#include "diagr.h"

/**
 * Version string of KmPlot.
 */
#define	KP_VERSION	"1.0"

/** The parser instance */
extern XParser	ps;

/** Diagram mode */
extern int mode;

/** If \p koordx is 4 (custom) this string will be evaluated by the Parser \p ps,
 * and the result is stored in \p xmin. */
extern QString	xminstr,
	/** If \p koordx is 4 (custom) this string will be evaluated by the Parser \p ps,
	 * and the result is stored in \p xmax. */
	xmaxstr,
	/** If \p koordy is 4 (custom) this string will be evaluated by the Parser \p ps,
	 * and the result is stored in \p ymin. */
	yminstr,
	/** If \p koordy is 4 (custom) this string will be evaluated by the Parser \p ps,
	 * and the result is stored in \p ymin. */
	ymaxstr;
//@}

extern double sw,  			/**< Precision for the functions.  */
	rsw,  /**< Precision for the grid. */

	/** @name Axes Division
	*
	* @see SkalDlg */ 
	//@{
	/** Distance between two tics on the x-axis or the distance between vertical grid lines. */
	tlgx,
	/** Distance between two tics on the y-axis or the distance between horizontal grid lines. */
	tlgy,
	//@}
	/** @name Printing Scale
	 *
	 * @see SklDlg */ 
	//@{
	/** 1 unit on the x-axis results \p drskalx cm at printing. */
	drskalx,
	/** 1 unit on the y-axis results \p drskaly cm at printing. */
	drskaly;
//@}

/** Current filename */
extern QString //dataFile,
	/**
	 * Axes tic step - string representation. 
	 * @see tlgx, tlgy
	 */
	tlgxstr,
	tlgystr,
	/**
	 * Printing unit - string representation. 
	 * @see drskalx, drskaly
	 */
	drskalxstr,
	drskalystr;

/** Header table printing option
 * @see KPinterDlg
 */
extern bool printtable;
 
/** Inits all global variables with respect to the global config file @p kc. 
 */
void getSettings();

/** Inits all global variables with respect to the global config file @p kc. 
 * Resetes the parser @p ps.
 */
void init();

#endif	// misc_included
