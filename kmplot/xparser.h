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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
	/// Evaluates the 1st dreivative of the function with intex \a ix
	double a1fkt( Equation * u_item , double, double h = 1e-3 );
	/// Evaluates the 2nd dreivative of the function with intex \a ix
	double a2fkt( Equation *, double, double h = 1e-3 );
	/// calculate euler's method when drawing a numeric prime-function
	double euler_method(const double, Equation * eq );
        
	/// Line width default
	QColor defaultColor(int function);
	
	virtual int addfkt( QString, QString, Function::Type type );
	
	///Returns an unused function name if it is needed
	void fixFunctionName(QString &, Equation::Type const = Equation::Cartesian, int const=-1);
        
        /// Interpretates the extended function string (only used by the old file format)
	bool getext( Function *, const QString );
	
	/// Functions for the DCOP interface:

	/// Returns a list with all functions
	QStringList listFunctionNames();

	/// Returns true if the graph is visible, otherwise false.
	bool functionFVisible(uint id);
	bool functionF1Visible(uint id);
	bool functionF2Visible(uint id);
	bool functionIntVisible(uint id);
	/// Set the visible of the function. Returns true if it succeeds, otherwise false.
	bool setFunctionFVisible(bool visible, uint id);
	bool setFunctionF1Visible(bool visible, uint id);
	bool setFunctionF2Visible(bool visible, uint id);
	bool setFunctionIntVisible(bool visible, uint id);
	
	/// Returns the function expression, or an empty string if the function couldn't be found
	QString functionStr(uint id, uint eq);
	/// Returns the complete function string including the extensions of a function, or an empty string if the function couldn't be found
	
	/// Get the color of a graph
	QColor functionFColor(uint id);
	QColor functionF1Color(uint id);
	QColor functionF2Color(uint id);
	QColor functionIntColor(uint id);
	/// Set the color of a graph. Returns true if it succeeds, otherwise false.
	bool setFunctionFColor(const QColor &color, uint id);
	bool setFunctionF1Color(const QColor &color, uint id);
	bool setFunctionF2Color(const QColor &color, uint id);
	bool setFunctionIntColor(const QColor &color, uint id);
	
	/// Get the line width of a graph
	double functionFLineWidth(uint id);
	double functionF1LineWidth(uint id);
	double functionF2LineWidth(uint id);
	double functionIntLineWidth(uint id);
	/// Set the line width of a graph. Returns true if it succeeds, otherwise false.
	bool setFunctionFLineWidth(double linewidth, uint id);
	bool setFunctionF1LineWidth(double linewidth, uint id);
	bool setFunctionF2LineWidth(double linewidth, uint id);
	bool setFunctionIntLineWidth(double linewidth, uint id);
		
	/// Returns the function's parameter list
	QStringList functionParameterList(uint id);
	bool functionAddParameter(const QString &new_parameter, uint id);
	bool functionRemoveParameter(const QString &remove_parameter, uint id);
	int addFunction(const QString &f_str0, const QString &f_str1);
	bool addFunction(const QString &extstr0, const QString &extstr1, bool f_mode, bool f1_mode, bool f2_mode, bool integral_mode, bool integral_use_precision, double linewidth, double f1linewidth, double f2linewidth, double integrallinewidth, const QString &str_dmin, const QString &str_dmax, const QString &str_startx, const QString &str_starty, double integral_precision, QColor color, QColor f1_color, QColor f2_color, QColor integral_color, QStringList str_parameter, int use_slider);
	bool setFunctionExpression(const QString &f_str, uint id, uint eq);
	
	/// Get the min and max value of a graph
	QString functionMinValue(uint id);
	QString functionMaxValue(uint id);
	/// Set the min and max values of a graph. Returns true if it succeeds, otherwise false.
	bool setFunctionMinValue(const QString &min, uint id);
	bool setFunctionMaxValue(const QString &max, uint id);
	
	/// Get the startx and starty value of a graph
	QString functionStartXValue(uint id);
	QString functionStartYValue(uint id);
	/// Set the startx and starty values of a graph. Returns true if it succeeds, otherwise false.
	bool setFunctionStartXValue(const QString &x, uint id);
	bool setFunctionStartYValue(const QString &y, uint id);
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
