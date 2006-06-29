/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
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
		static XParser * self( bool * modified = 0 );
		
		~XParser();
		/**
		 * Evaluates the \p n th derivative of the equation using numerical
		 * stepsize \p h.
		 */
		double derivative( int n, Equation * eq, double x, double h );
		/**
		 * For use with functions of two variables.
		 */
		double partialDerivative( int n1, int n2, Equation * eq, double x, double y, double h1, double h2 );
		/**
		 * Calculates the value of the equation using numerical integration
		 * with the given step size \p h (which is only used as a hint).
		 */
		double integral( Equation * eq, double x, double h );
		/**
		 * The settings contain 10 default function colors. This returns the
		 * (function % 10)th color.
		 */
		QColor defaultColor(int function);
	
	virtual int addFunction( QString, QString, Function::Type type );
	
	///Returns an unused function name if it is needed
	void fixFunctionName(QString &, Equation::Type const = Equation::Cartesian, int const=-1);
        
        /// Interpretates the extended function string (only used by the old file format)
	bool getext( Function *, const QString );
	
	/// Functions for the D-BUS interface:
    public Q_SLOTS:
	/// Returns a list with all functions
	Q_SCRIPTABLE QStringList listFunctionNames();

	/// Returns true if the graph is visible, otherwise false.
	Q_SCRIPTABLE bool functionFVisible(uint id);
	Q_SCRIPTABLE bool functionF1Visible(uint id);
	Q_SCRIPTABLE bool functionF2Visible(uint id);
	Q_SCRIPTABLE bool functionIntVisible(uint id);
	/// Set the visible of the function. Returns true if it succeeds, otherwise false.
	Q_SCRIPTABLE bool setFunctionFVisible(bool visible, uint id);
	Q_SCRIPTABLE bool setFunctionF1Visible(bool visible, uint id);
	Q_SCRIPTABLE bool setFunctionF2Visible(bool visible, uint id);
	Q_SCRIPTABLE bool setFunctionIntVisible(bool visible, uint id);
	
	/// Returns the function expression, or an empty string if the function couldn't be found
	Q_SCRIPTABLE QString functionStr(uint id, uint eq);
	/// Returns the complete function string including the extensions of a function, or an empty string if the function couldn't be found
	
	/// Get the color of a graph
	Q_SCRIPTABLE QColor functionFColor(uint id);
	Q_SCRIPTABLE QColor functionF1Color(uint id);
	Q_SCRIPTABLE QColor functionF2Color(uint id);
	Q_SCRIPTABLE QColor functionIntColor(uint id);
	/// Set the color of a graph. Returns true if it succeeds, otherwise false.
	Q_SCRIPTABLE bool setFunctionFColor(const QColor &color, uint id);
	Q_SCRIPTABLE bool setFunctionF1Color(const QColor &color, uint id);
	Q_SCRIPTABLE bool setFunctionF2Color(const QColor &color, uint id);
	Q_SCRIPTABLE bool setFunctionIntColor(const QColor &color, uint id);
	
	/// Get the line width of a graph
	Q_SCRIPTABLE double functionFLineWidth(uint id);
	Q_SCRIPTABLE double functionF1LineWidth(uint id);
	Q_SCRIPTABLE double functionF2LineWidth(uint id);
	Q_SCRIPTABLE double functionIntLineWidth(uint id);
	/// Set the line width of a graph. Returns true if it succeeds, otherwise false.
	Q_SCRIPTABLE bool setFunctionFLineWidth(double linewidth, uint id);
	Q_SCRIPTABLE bool setFunctionF1LineWidth(double linewidth, uint id);
	Q_SCRIPTABLE bool setFunctionF2LineWidth(double linewidth, uint id);
	Q_SCRIPTABLE bool setFunctionIntLineWidth(double linewidth, uint id);
		
	/// Returns the function's parameter list
	Q_SCRIPTABLE QStringList functionParameterList(uint id);
	Q_SCRIPTABLE bool functionAddParameter(const QString &new_parameter, uint id);
	Q_SCRIPTABLE bool functionRemoveParameter(const QString &remove_parameter, uint id);
	Q_SCRIPTABLE int addFunction(const QString &f_str0, const QString &f_str1);
	Q_SCRIPTABLE bool addFunction(const QString &extstr0, const QString &extstr1, bool f_mode, bool f1_mode, bool f2_mode, bool integral_mode, bool Q_SCRIPTABLE integral_use_precision, double linewidth, double f1linewidth, double f2linewidth, double integrallinewidth, const QString &str_dmin, const QString &str_dmax, const QString &str_startx, const QString &str_starty, double integral_precision, QColor color, QColor f1_color, QColor f2_color, QColor integral_color, QStringList str_parameter, int use_slider);
	Q_SCRIPTABLE bool setFunctionExpression(const QString &f_str, uint id, uint eq);
	
	/// Get the min and max value of a graph
	Q_SCRIPTABLE QString functionMinValue(uint id);
	Q_SCRIPTABLE QString functionMaxValue(uint id);
	/// Set the min and max values of a graph. Returns true if it succeeds, otherwise false.
	Q_SCRIPTABLE bool setFunctionMinValue(const QString &min, uint id);
	Q_SCRIPTABLE bool setFunctionMaxValue(const QString &max, uint id);
	
	/// Get the startx and starty value of a graph
	Q_SCRIPTABLE QString functionStartXValue(uint id);
	Q_SCRIPTABLE QString functionStartYValue(uint id);
	/// Set the startx and starty values of a graph. Returns true if it succeeds, otherwise false.
	Q_SCRIPTABLE bool setFunctionStartValue(const QString &x, const QString &y, uint id);
private:
        
	/// finds a free function name 
	void findFunctionName(QString &, int const, int const);
	/// indicates if the widget is changed
	bool & m_modified;
	
	
private:
	XParser( bool & modified );
	static XParser * m_self;
};

#endif //xparser_included
