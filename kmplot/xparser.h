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

#ifndef xparser_included
#define xparser_included

// Qt includes
#include <qcolor.h>
#include <qstring.h>

// local includes
#include "parser.h"

/**
 * @short Extended parser class.
 *
 * This class extends the parser class to support derivatives, 
 * draw options like color and line width and so on.
 */
class XParser : public Parser
{
public:
	XParser();
	XParser( int anz, int m_size, int s_size );
	~XParser();


	/// Interpretats the extended function string
	int getext( int );
	/// Removes the function with index \a ix from the parser.
	int delfkt( int ix );
	/// Evaluates the 1st dreivative of the function with intex \a ix
	double a1fkt( int ix , double, double h = 1e-3 );
	/// Evaluates the 2nd dreivative of the function with intex \a ix
	double a2fkt( int, double, double h = 1e-3 );
	
	/// Line width default
	int dicke0;

	/// Extended attributes are encapulated in this structure.
	struct FktExt
	{
		char f_mode, ///< \a f_mode == 1: draw the plot.
		f1_mode, ///< \a f1_mode == 1.  draw the 1st derivative, too.
		f2_mode;///< \a f2_mode == 1.  draw the 2nd derivative, too.
		int dicke, ///< Line width.
		/** Number of parameter values. 
		 * @see FktExt::k_liste */
		k_anz; 
		QString str_dmin, str_dmax; /// Plot range, input strings.
		double dmin, ///< Custom plot range, lower boundage.
		dmax, ///< Custom plot range, upper boundage.
		/** List of parameter values. 
		 * @see FktExt::k_anz */
		k_liste[ 10 ];
		QString extstr; ///< Complete function string including the extensions.
		QRgb color, ///< current color.
		color0; ///< Default color.
	}
	*fktext;
};

#endif //xparser_included

