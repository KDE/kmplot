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
	XParser(bool &);
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
	
	/// Send a function to an other instance of Kmplot. Returns true if it success, otherwise false
	bool sendFunction(int id);
	
	/// Functions for the DCOP interface:

	/// Returns a list with all functions
	QStringList listFunctionNames();

	/// Returns true if the function is visible, otherwise false
	bool functionFVisible(uint id);
	/// Returns true if the first derivative of the function is visible, otherwise false
	bool functionF1Visible(uint id);
	/// Returns true if the second derivative of the function is visible, otherwise false
	bool functionF2Visible(uint id);
	/// Returns true if the integral of the function is visible, otherwise false
	bool functionIntVisible(uint id);
	/// Returns the expression of a function, or an empty string if the function couldn't be found
	QString functionFstr(uint id);
	/// Returns the complete function string including the extensions of a function, or an empty string if the function couldn't be found
	QString functionExtstr(uint id);

	bool setFunctionFVisible(bool visible, uint id);
	bool setFunctionF1Visible(bool visible, uint id);
	bool setFunctionF2Visible(bool visible, uint id);
	bool setFunctionIntVisible(bool visible, uint id);
	
	QColor functionFColor(uint id);
	QColor functionF1Color(uint id);
	QColor functionF2Color(uint id);
	QColor functionIntColor(uint id);
	bool setFunctionFColor(const QColor &color, uint id);
	bool setFunctionF1Color(const QColor &color, uint id);
	bool setFunctionF2Color(const QColor &color, uint id);
	bool setFunctionIntColor(const QColor &color, uint id);
	
	int functionFLineWidth(uint id);
	int functionF1LineWidth(uint id);
	int functionF2LineWidth(uint id);
	int functionIntLineWidth(uint id);
	bool setFunctionFLineWidth(int linewidth, uint id);
	bool setFunctionF1LineWidth(int linewidth, uint id);
	bool setFunctionF2LineWidth(int linewidth, uint id);
	bool setFunctionIntLineWidth(int linewidth, uint id);
		
	/// Returns the function's parameter list
	QStringList functionParameterList(uint id);
	bool functionAddParameter(const QString &new_parameter, uint id);
	bool functionRemoveParameter(const QString &remove_parameter, uint id);
	int addFunction(const QString &f_str);
	bool addFunction(const QString &extstr, bool f_mode, bool f1_mode, bool f2_mode, bool integral_mode, bool integral_use_precision, int linewidth, int f1_linewidth, int f2_linewidth, int integral_linewidth, const QString &str_dmin, const QString &str_dmax, const QString &str_startx, const QString &str_starty, double integral_precision, QRgb color, QRgb f1_color, QRgb f2_color, QRgb integral_color, QStringList str_parameter, bool use_slider);
	bool setFunctionExpression(const QString &f_str, uint id);
	
private:
        
	/// finds a free function name 
	void findFunctionName(QString &, int const, int const);
	/// used when sending parametric functions
	QString m_tmpdcopclient; 
	/// indicates if the widget is changed
	bool &m_modified;
};

#endif //xparser_included
