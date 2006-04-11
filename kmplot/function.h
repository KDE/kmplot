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
#include <QPointF>
#include <QString>


class Equation;
class Function;


/**
 * This stores a string which evaluates directly to a number (i.e. without any
 * input variables such as x).
 */
class Value
{
	public:
		Value( const QString & expression = QString() );
		
		/**
		 * @return The value of the current expression.
		 */
		double value() const { return m_value; }
		/**
		 * @return The current expression.
		 */
		QString expression() const { return m_expression; }
		/**
		 * Sets the current expression. If the expression could be evaluated
		 * (i.e. no errors), then value is updated, the expression is saved and
		 * true is returned. Otherwise, just returns false.
		 */
		bool updateExpression( const QString & expression );
		/**
		 * This checks if the expression strings (and hence values) are
		 * identical.
		 */
		bool operator == ( const Value & other );
		/**
		 * Checks for inequality.
		 */
		bool operator != ( const Value & other ) { return !((*this) == other); }
		
	protected:
		QString m_expression;
		double m_value;
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


/**
 * This is the non-visual mathematical expression.
 */
class Equation
{
	public:
		enum Type
		{
			Cartesian,
			ParametricX,
			ParametricY,
			Polar,
		};
		
		Equation( Type type, Function * parent );
		~Equation();
		
		/// The type of function
		Type type() const { return m_type; }
		/**
		 * \return whether this Equation has different user-entered values to
		 * the \p other equation.
		 */
		bool operator != ( const Equation & other );
		/**
		 * Assigns the value in \p other to this equation.
		 */
		Equation & operator = ( const Equation & other );
		/**
		 * Pointer to the allocated memory for the tokens.
		 */
		unsigned char *mem;
		/**
		 * Pointer to the token.
		 */
		unsigned char *mptr;
		/**
		 * @return a pointer to Function parent of this Equation.
		 */
		Function * parent() const { return m_parent; }
		/**
		 * @return the name of the function, e.g. for the cartesian function
		 * f(x)=x^2, this would return "f".
		 */
		QString fname() const;	///< Name of the function.
		/**
		 * @return the dummy variable of the function, e.g. for the cartesian
		 * function f(x,k)=(x+k)(x-k), this would return "x".
		 */
		QString fvar() const;
		/**
		 * @return the parameter if one exists, e.g. for the cartesian function
		 * f(x,k)=(x+k)(x-k), this would return "k". For the function f(x)=x^2,
		 * this would return an empty string.
		 */
		QString fpar() const;
		/**
		 * The full function expression, e.g. "f(x,k)=(x+k)(x-k)".
		 */
		QString fstr() const { return m_fstr; }
		/**
		 * @see fstr()
		 * @return whether \p fstr could be parsed correctly. Note that if it
		 * was not parsed correctly, then this will return false and this class
		 * will not be updated. If \p force is true, then no checks on the
		 * validity of the string are performed.
		 */
		bool setFstr( const QString & fstr, bool force = false );
		/**
		 * Sets the start position and value of the integral.
		 */
		void setIntegralStart( const Value & x, const Value & y );
		/**
		 * The initial x-value in calculating integrals.
		 */
		Value integralInitialX() const { return m_startX; }
		/**
		 * The initial y-value in calculating integrals.
		 */
		Value integralInitialY() const { return m_startY; }
		/**
		 * Resets lastIntegralPoint  to the initial integral point.
		 */
		void resetLastIntegralPoint();
		
		QPointF lastIntegralPoint; ///< needed for numeric integration
		
		double oldy; /// \todo is this value needed?
		
	protected:
		/// \note when adding new member variables, make sure to update operator != and operator =
		const Type m_type;
		QString m_fstr;
		Function * m_parent;
		Value m_startX, m_startY;
};



/** Here are all atitrbutes for a function. */
class Function
{
	public:
		enum PMode
		{
			Derivative0,
			Derivative1,
			Derivative2,
			Integral,
		};
		
		enum Type
		{
			Cartesian,
			Parametric,
			Polar,
		};
		
		Function( Type type );
		~Function();
		
		/// The type of function
		Type type() const { return m_type; }
		/**
		 * \return A string for displaying to the user that identifies this
		 * equation, taking into account \p mode.
		 */
		QString prettyName( Function::PMode mode ) const;
		/**
		 * Converts the type to a string (which is used in save files).
		 */
		static QString typeToString( Type type );
		/**
		 * Converts the string to a type (used when loading files).
		 */
		static Type stringToType( const QString & type );
		/**
		 * Sets the parameter.
		 */
		void setParameter( double p ) { k = p; };
		/**
		 * The function parameter, as set by e.g. a slider.
		 */
		double k;
		
		/**
		 * A list with all functions this function depends on.
		 */
		QList<int> dep;
		
		/**
		 * Copies data members across, while avoiding id, mem, mptr type
		 * variables.
		 * @return whether any values have changed.
		 */
		bool copyFrom( const Function & function );
        
		uint id;
		Equation * eq[2];
		
		Plot f0;		///< The actual function - the "zero'th derivative"
		Plot f1;		///< First derivative
		Plot f2;		///< Second derivative
		Plot integral;	///< integral
		
		/**
		 * The user can specify an unique precision for numeric prime-functions.
		 */
		bool integral_use_precision:1;
		/**
		 * Custom plot range, lower boundary.
		 */
		Value dmin;
		/**
		 * Custom plot range, upper boundary.
		 */
		Value dmax;
		/**
		 * -1: None (use list)
		 * else: slider number.
		 */
		int use_slider;
		/**
		 * List with parameter for the function.
		 */
		QList<Value> parameters;
		bool usecustomxmin:1;
		bool usecustomxmax:1;
		double integral_precision;
		// TODO double slider_min, slider_max; ///< extreme values of the slider
		
	protected:
		const Type m_type;
};


#endif	// FUNCTION_H
