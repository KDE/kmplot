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


/** The global Config file */
extern KConfig *kc;

/** The parser instance */
extern XParser	ps;

/** Diagram mode */
extern int mode;
/** Grid mode */
extern int g_mode;


/** Thickness of the axes lines. */
extern int	axesThickness,
	/** Thickness of the grid lines. */
	gridThickness,
	/** Thickness of the tics */
	gradThickness,
	/** Length of the tics. */
	gradLength;

/** @name Limits of the coordinate axes
 * These are variables for the handling of the axes' limits.
 *
 * @p koordx and @p koordy can have the values 0 to 4 which have the following meanings: 
 * @li 0: -8..8
 * @li 1: -5..5
 * @li 2: 0..16
 * @li 3: 0..10
 * @li 4: custom
 * 
 * In the last case @p xminstr, @p xmaxstr, @p yminstr, and @p ymaxstr are evaluates.
 *
 * @see coordToMinMax()
 */ 
//@{
extern	int koordx,
	koordy;
/** Left edge of the x-axis. */
extern	double xmin,
	/** Right edge of the x-axis. */
	xmax,
	/** Bottom edge of the y-axis. */
	ymin,
	/** Top edge of the y-axis. */
	ymax;
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

/**
 * Font family name for the header table.
 * @see KPrinterDlg
 */
extern QString font_header,
	/**
	 * Font family name for the axes' labels.
	 * @see KPrinterDlg
	 */
	font_axes;

/** Color of the axes. */
extern	QRgb axesColor,
	/** Color of the grid. */
	gridColor;

/** Header table printing option
 * @see KPinterDlg
 */
extern bool printtable;
 
/**
 * Converting 
 */
bool coordToMinMax( const int koord, double &min, double &max, const QString minStr = "", const QString maxStr = "" );

/** Inits all global variables with respect to the global config file @p kc. 
 */
void getSettings();

/** Inits all global variables with respect to the global config file @p kc. 
 * Resetes the parser @p ps.
 */
void init();

/**
 * Evaluation of the predefined axes settings (kkordx/y).
 * @result true, if the evaluation of the strings was successful.
 */
bool coordToMinMax( const int koord, double &min, double &max, const QString minStr, const QString maxStr );

#endif	// misc_included
