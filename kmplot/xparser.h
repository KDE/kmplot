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

#define SLIDER_COUNT 4

// Qt includes
#include <kdebug.h>

// local includes
#include "parser.h"
#include "settings.h"

/**
 * @short Extended parser class.
 *
 * This class extends the parser class to support derivatives, 
 * draw options like color and line width and so on.
 */
class XParser : public Parser
{
public:
	XParser(int m_size, int s_size );
	~XParser();
	/// Evaluates the 1st dreivative of the function with intex \a ix
	double a1fkt( Ufkt *u_item , double, double h = 1e-3 );
	/// Evaluates the 2nd dreivative of the function with intex \a ix
	double a2fkt( Ufkt *, double, double h = 1e-3 );
        /// calculate euler's method when drawing a numeric prime-function
        double euler_method(const double, const QValueVector<Ufkt>::iterator);
        
        /// Line width default
        int linewidth0;
        QRgb defaultColor(int function);
	
        enum { Function, Polar, Parametric }; ///types of functions
	///Returns an unused function name if it is needed
	void fixFunctionName(QString &, int const = XParser::Function , int const=-1);
        
        /// Returns the index for the next function.
        int getNextIndex();
        /// Fill item with default values (e.g color, linewidth )
        void prepareAddingFunction(Ufkt *item);
        
        /// Interpretates the extended function string
        int getext( Ufkt * );
        
private:
        
	/// finds a free function name 
	void findFunctionName(QString &, int const, int const);
};

#endif //xparser_included
