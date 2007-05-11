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
#include "vector.h"

/**
 * @short Extended parser class.
 *
 * This class extends the parser class to support derivatives, 
 * draw options like color and line width and so on.
 */
class XParser : public Parser
{
	public:
		static XParser * self();
		
		~XParser();
		/**
		 * Evaluates the \p n th derivative of the equation using numerical
		 * stepsize \p h.
		 */
		double derivative( int n, Equation * eq, DifferentialState * state, double x, double h );
		/**
		 * For use with functions of two variables.
		 */
		double partialDerivative( int n1, int n2, Equation * eq, DifferentialState * state, double x, double y, double h1, double h2 );
		/**
		 * For differential equations - uses numerical integration to
		 * calculate value for the given x. Differential equations often have
		 * the annoying habbit of diverging to infinity rapidly. If this
		 * happens while trying to calculate the value, then
		 * XParser::differentialFinite will be set to false, and 
		 * XParserdifferentialDiverge will be set to the last point where the
		 * differential value was finite.
		 */
		double differential( Equation * eq, DifferentialState * state, double x, double max_dx );
		bool differentialFinite;
		double differentialDiverge;
		/**
		 * The settings contain 10 default function colors. This returns the
		 * (function % 10)th color.
		 */
		QColor defaultColor(int function);
	
		/// finds a free function name 
		QString findFunctionName( const QString & preferredName, int id );
		
		///Returns an unused function name if it is needed
		void fixFunctionName(QString &, Equation::Type const = Equation::Cartesian, int const=-1);
        
        /// Interpretates the extended function string (only used by the old file format)
	bool getext( Function *, const QString &);
	
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
	Q_SCRIPTABLE bool setFunctionFVisible(uint id, bool visible);
	Q_SCRIPTABLE bool setFunctionF1Visible(uint id, bool visible);
	Q_SCRIPTABLE bool setFunctionF2Visible(uint id, bool visible);
	Q_SCRIPTABLE bool setFunctionIntVisible(uint id, bool visible);
	
	/// Returns the function expression, or an empty string if the function couldn't be found
	Q_SCRIPTABLE QString functionStr(uint id, uint eq);
	/// Returns the complete function string including the extensions of a function, or an empty string if the function couldn't be found
	
	/// Get the color of a graph
	Q_SCRIPTABLE QColor functionFColor(uint id);
	Q_SCRIPTABLE QColor functionF1Color(uint id);
	Q_SCRIPTABLE QColor functionF2Color(uint id);
	Q_SCRIPTABLE QColor functionIntColor(uint id);
	/// Set the color of a graph. Returns true if it succeeds, otherwise false.
	Q_SCRIPTABLE bool setFunctionFColor(uint id, const QColor &color);
	Q_SCRIPTABLE bool setFunctionF1Color(uint id, const QColor &color);
	Q_SCRIPTABLE bool setFunctionF2Color(uint id, const QColor &color);
	Q_SCRIPTABLE bool setFunctionIntColor(uint id, const QColor &color);
	
	/// Get the line width of a graph
	Q_SCRIPTABLE double functionFLineWidth(uint id);
	Q_SCRIPTABLE double functionF1LineWidth(uint id);
	Q_SCRIPTABLE double functionF2LineWidth(uint id);
	Q_SCRIPTABLE double functionIntLineWidth(uint id);
	/// Set the line width of a graph. Returns true if it succeeds, otherwise false.
	Q_SCRIPTABLE bool setFunctionFLineWidth(uint id, double linewidth);
	Q_SCRIPTABLE bool setFunctionF1LineWidth(uint id, double linewidth);
	Q_SCRIPTABLE bool setFunctionF2LineWidth(uint id, double linewidth);
	Q_SCRIPTABLE bool setFunctionIntLineWidth(uint id, double linewidth);
		
	/// Returns the function's parameter list
	Q_SCRIPTABLE QStringList functionParameterList(uint id);
	Q_SCRIPTABLE bool functionAddParameter(uint id, const QString &new_parameter);
	Q_SCRIPTABLE bool functionRemoveParameter(uint id, const QString &remove_parameter);
	Q_SCRIPTABLE int addFunction(const QString &f_str0, const QString &f_str1);
	Q_SCRIPTABLE bool addFunction(const QString &extstr0, const QString &extstr1, bool f_mode, bool f1_mode, bool f2_mode, bool integral_mode, double linewidth, double f1linewidth, double f2linewidth, double integrallinewidth, const QString &str_dmin, const QString &str_dmax, const QString &str_startx, const QString &str_starty, double integral_precision, const QColor &color, const QColor &f1_color, const QColor &f2_color, const QColor &integral_color, const QStringList & str_parameter, int use_slider);
	Q_SCRIPTABLE bool setFunctionExpression(uint id, uint eq, const QString &f_str);
	
	/// Get the min and max value of a graph
	Q_SCRIPTABLE QString functionMinValue(uint id);
	Q_SCRIPTABLE QString functionMaxValue(uint id);
	/// Set the min and max values of a graph. Returns true if it succeeds, otherwise false.
	Q_SCRIPTABLE bool setFunctionMinValue(uint id, const QString &min);
	Q_SCRIPTABLE bool setFunctionMaxValue(uint id, const QString &max);
	
	/// Get the startx and starty value of a graph
	Q_SCRIPTABLE QString functionStartXValue(uint id);
	Q_SCRIPTABLE QString functionStartYValue(uint id);
	/// Set the startx and starty values of a graph. Returns true if it succeeds, otherwise false.
	Q_SCRIPTABLE bool setFunctionStartValue(uint id, const QString &x, const QString &y);
	
	private:
		Vector rk4_f( int order, Equation * eq, double x, const Vector & y );
		/// for use in differential
		Vector m_k1, m_k2, m_k3, m_k4, m_y_temp, m_y, m_result, m_arg;
	
	
private:
	XParser();
	static XParser * m_self;
};

#endif //xparser_included
