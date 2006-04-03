/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
*               2000, 2002 kd.moeller@t-online.de
*                     2006 David Saxton <david@bluehaze.org>
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#ifndef FUNCTION_H
#define FUNCTION_H

#include <QColor>
#include <QString>


/// A parameter expression and value
class ParameterValueItem
{
	public:
		ParameterValueItem(const QString &e, double v)
		{
			expression = e;
			value = v;
		};
		ParameterValueItem() {;};
		QString expression;
		double value;
		
		bool operator==( const ParameterValueItem & vi )
		{
			return (vi.value == value) && (vi.expression == expression);
		};
};



/**
 * Stores details of a plot of a function (e.g. its derivative or integral).
 */
class Plot
{
	public:
		Plot();
		
		double lineWidth;	///< line width in mm
		QColor color;		///< color that the plot will be drawn in
		bool visible;		///< whether to display this plot
		
		bool operator != ( const Plot & other ) const;
};



/** Here are all atitrbutes for a function stored. */
class Ufkt
{
	public:
		enum PMode
		{
			Function,
			Derivative1,
			Derivative2,
			Integral,
		};
		
		Ufkt();
		~Ufkt();
		
		/**
		 * Copies data members across, while avoiding id, mem, mptr type
		 * variables.
		 * @return whether any values have changed.
		 */
		bool copyFrom( const Ufkt & function );
		
		/// Sets the parameter 
		void setParameter( double p ) { k = p; };
        
		uint id;
		unsigned char *mem;     ///< Pointer to the allocated memory for the tokens.
		unsigned char *mptr;    ///< Pointer to the token.
		QString fname;          ///< Name of the function.
		QString fvar;           ///< Dummy variable.
		QString fpar;           ///< Parameter.
		QString fstr;           ///< Function expression.
		double k,               ///< Function parameter.
		oldy;                   ///< The last y-value needed for Euler's method
		QList<int> dep;   /// A list with all functions this function depends on
        
		Plot f0;		///< The actual function - the "zero'th derivative"
		Plot f1;		///< First derivative
		Plot f2;		///< Second derivative
		Plot integral;	///< integral
		
		bool integral_use_precision:1; ///< The user can specify an unic precision for numeric prime-functions
		/** Number of parameter values. 
		 * @see FktExt::k_liste */
		QString str_dmin, str_dmax, str_startx, str_starty ; /// Plot range, input strings.
		double dmin, ///< Custom plot range, lower boundage.
		dmax, ///< Custom plot range, upper boundage.
		/** List of parameter values. 
		 * @see FktExt::k_anz */
		oldyprim,  ///< needed for Euler's method, the last y'.value
		oldx, ///< needed for Euler's method, the last x-value
		starty,///< startposition for Euler's method, the initial y-value
		startx, ///< startposition for Euler's method, the initial x-value last y'.valuenitial x-value last y'.valuenitial x-value
		integral_precision; ///<precision when drawing numeric prime-functions
		int use_slider; ///< -1: none (use list), else: slider number
		QList<ParameterValueItem> parameters; ///< List with parameter for the function
		bool usecustomxmin:1;
		bool usecustomxmax:1;
        	// TODO double slider_min, slider_max; ///< extreme values of the slider
};


#endif	// FUNCTION_H
