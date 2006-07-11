/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter M�ler <kd.moeller@t-online.de>
*                                 2006  David Saxton <david@bluehaze.org>
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

#include "vector.h"

#include <QColor>
#include <QFlags>
#include <QGradient>
#include <QPointF>
#include <QString>
#include <QVector>

class Equation;
class Function;
class Plot;


/**
 * Maximum number of plus-minus-s; each added one doubles the number of combinations
 * which quickly grows. So put a bound on the number allowed.
 */
extern int MAX_PM; 


/**
 * This stores a string which evaluates directly to a number (i.e. without any
 * input variables such as x).
 */
class Value
{
	public:
		/**
		 * Initializes Value with \p expression.
		 * This will have the value 0 if \p expression is empty or unparsable.
		 */
		Value( const QString & expression = QString() );
		/**
		 * Converts \p value to a string (see Parser::number) and initializes
		 * this with the \p value.
		 */
		explicit Value( double value );
		
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
		 * Converts \p value to a string (see Parser::number) and uses it for
		 * the current expression.
		 */
		void updateExpression( double value );
		/**
		 * This checks if the expression strings (and hence values) are
		 * identical.
		 */
		bool operator == ( const Value & other ) const;
		/**
		 * Checks for inequality.
		 */
		bool operator != ( const Value & other ) const { return !((*this) == other); }
		
	protected:
		QString m_expression;
		double m_value;
};


/**
 * Stores details of the appearance of a plot of a function (e.g. its
 * derivative or integral).
 */
class PlotAppearance
{
	public:
		PlotAppearance();
		
		// NOTE: When adding more members to this class, remember to update
		// the function PlotAppearance::operator!= and the functions in
		// KmPlotIO for saving / loading the plot appearance

		double lineWidth;		///< line width in mm
		QColor color;			///< color that the plot will be drawn in
		Qt::PenStyle style;		///< pen style (e.g. dolif, dashes, dotted, etc)
		QGradient gradient;		///< the gradient if useGradient is true
		bool useGradient:1;		///< for plots with parameters, whether to use gradient instead of color
		bool showExtrema:1;		///< for cartesian functions, whether to show the extreme values of the function
		bool showTangentField:1;///< whether to draw the tangent field (for differential equations
		bool visible:1;			///< whether to display this plot
		
		bool operator != ( const PlotAppearance & other ) const;
		
		/**
		 * Converts a pen style to a string (for non-displaying uses such as
		 * saving to file).
		 */
		static QString penStyleToString( Qt::PenStyle style );
		/**
		 * Converts a string (as returned by penStyleTostring) to a pen style.
		 */
		static Qt::PenStyle stringToPenStyle( const QString & style );
};


/**
 * Used in differential equations; contains the initial conditions and the
 * currently calculated value (used as a cache).
 */
class DifferentialState
{
	public:
		DifferentialState();
		DifferentialState( int order );
		
		/**
		 * Resizes y, y0. Also calls resetToInitial.
		 */
		void setOrder( int order );
		/**
		 * Sets y=y0, x=x0.
		 */
		void resetToInitial();
		
		Value x0; ///< the initial x-value
		QVector<Value> y0; ///< the value of ( f, f', f'', ...., f^(n) ) at x0
		double x; ///< the current x value
		Vector y; ///< the value of ( f, f', f'', ...., f^(n) ) at x
		
		/**
		 * Whether the initial conditions and current state are the same.
		 */
		bool operator == ( const DifferentialState & other ) const;
};


class DifferentialStates
{
	public:
		DifferentialStates();
		
		/**
		 * \see order()
		 */
		void setOrder( int order );
		/**
		 * Creates a differential state.
		 */
		DifferentialState * add();
		/**
		 * The order of the differential equations, e.g. "f''(x) = -f" is of
		 * order 2.
		 */
		int order() const { return m_order; }
		/**
		 * The number of differential states.
		 */
		int size() const { return m_data.size(); }
		/**
		 * Calls DifferentialState::resetToInitial for each state; i.e. resets
		 * the cached information about the state of the differential equation.
		 */
		void resetToInitial();
		/**
		 * The maximum step-size (and hence minimum precision) used in the RK4
		 * method (see XParser::differential). Of course, a smaller step size
		 * may be used in the visible section of a differential plot.
		 */
		Value step() const { return m_step; }
		/**
		 * \see maximumStep();
		 */
		void setStep( const Value & step ) { m_step = step; }
		
		bool operator == ( const DifferentialStates & other ) const { return (m_data == other.m_data) && (m_step == other.m_step); }
		bool operator != ( const DifferentialStates & other ) const { return !(*this == other); }
		DifferentialState & operator[] ( int i ) { return m_data[i]; }
		const DifferentialState & operator[] ( int i ) const { return m_data[i]; }
		void remove ( int i ) { m_data.remove(i); }
		void remove ( int i, int count ) { m_data.remove( i, count ); }
		
	protected:
		QVector<DifferentialState> m_data;
		int m_order;
		Value m_step;
};


/**
 * This is the non-visual mathematical expression.
 * \note when adding new member variables, make sure to update operator !=
 * and operator =.
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
			Implicit,
			Differential
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
		QString name( bool removePrimes = true ) const;
		/**
		 * \return a list of variables, e.g. {x} for "f(x)=y", and {x,y,k} for
		 * "f(x,y,k)=(x+k)(y+k)".
		 */
		QStringList variables() const { return m_variables; }
		/**
		 * The full function expression, e.g. "f(x,k)=(x+k)(x-k)".
		 */
		QString fstr() const { return m_fstr; }
		/**
		 * @see fstr()
		 * @param string the equation
		 * @param error if non-null, then will be set to the parser error (or
		 * success).
		 * @param errorPosition the error position
		 * @return whether \p fstr could be parsed correctly. Note that if it
		 * was not parsed correctly, then this will return false and this class
		 * will not be updated.
		 */
		bool setFstr( const QString & string, int * error = 0, int * errorPosition = 0 );
		/**
		 * \return the order of the differential equations.
		 */
		int order() const;
		/**
		 * \return the number of plus-minus symbols in the equation.
		 */
		int pmCount() const;
		
		/// For differential equations, all the states
		DifferentialStates differentialStates;
		
		/**
		 * The current plus-minus signature (true for plus, false for minus).
		 */
		QVector<bool> pmSignature() const { return m_pmSignature; }
		/**
		 * \see pmSignature.
		 */
		void setPMSignature( QVector<bool> pmSignature );
		
	protected:
		/**
		 * Updates m_variables.
		 */
		void updateVariables();
		
		const Type m_type;
		QString m_fstr;
		Function * m_parent;
		QVector<bool> m_pmSignature;
		/**
		 * Cached list of variables. Updated when setFstr is called.
		 */
		QStringList m_variables;
};


/**
 * Which parameters to use and how.
 */
 class ParameterSettings
{
	public:
		ParameterSettings();
		
		bool operator == ( const ParameterSettings & other ) const;
		bool operator != ( const ParameterSettings & other ) const { return !((*this) == other); }
		
		bool animating; ///< if true, then useSlider and useList are ignored, parameter value is assumed to be updated
		bool useSlider;
		int sliderID;
		bool useList;
		QList< Value > list;
};


/**
 * Uniquely identifies a parameter (which could be from the list of Values
 * stored in a Function or from a Slider.
 */
 class Parameter
{
	public:
		enum Type { Unknown, Animated, Slider, List };
		Parameter( Type type = Unknown );
		
		Type type() const { return m_type; }
		/**
		 * The slider ID specifies which slider to use (e.g. "2" specifies the
		 * third slider).
		 */
		void setSliderID( int id ) { m_sliderID = id; }
		/**
		 * The list pos specifies which parameter to use in the list
		 * ParameterSettings::list.
		 */
		void setListPos( int pos ) { m_listPos = pos; }
		/**
		 * \see setSliderID
		 */
		int sliderID() const { return m_sliderID; }
		/**
		 * \see setListPos
		 */
		int listPos() const { return m_listPos; }
		/**
		 * \return Whether the parameter referred to is the same.
		 */
		bool operator == ( const Parameter & other ) const;
		
	protected:
		Type m_type;
		int m_sliderID;
		int m_listPos;
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
			Integral
		};
		
		enum Type
		{
			Cartesian,
			Parametric,
			Polar,
			Implicit,
			Differential
		};
		
		Function( Type type );
		~Function();
		
		/**
		 * \return the type of function.
		 */
		Type type() const { return m_type; }
		
		enum PlotCombination
		{
			DifferentParameters		= 0x1,		///< For all the different parameters
			DifferentDerivatives	= 0x2,		///< Derivatives of the function
			DifferentPMSignatures	= 0x4,		///< Plus-minus combinations
			DifferentInitialStates	= 0x8,		///< For differential equations; different states
			AllCombinations			= 0x20-1
		};
		typedef QFlags<PlotCombination> PlotCombinations;
		
		/**
		 * \return a list of plots for this function, 
		 */
		QList< Plot > plots( PlotCombinations combinations = AllCombinations ) const;
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
		 * Sets the current working parameter (which is used in calculations).
		 */
		void setParameter( double p ) { k = p; };
		/**
		 * The function parameter, as set by e.g. a slider.
		 */
		double k;
		/**
		 * Clears the list of functions that this function depends on.
		 */
		void clearFunctionDependencies() { m_dependencies.clear(); }
		/**
		 * Adds \p function to the list of functions that this function depends
		 * on. For example, if this function is "f(x) = 1 + g(x)", then this
		 * function depends on the function g(x).
		 */
		void addFunctionDependency( Function * function );
		/**
		 * \return whether this function or any of the functions that this
		 * function depend on, etc, depend on \p function.
		 */
		bool dependsOn( Function * function ) const;
		/**
		 * Copies data members across, while avoiding id, mem, mptr type
		 * variables.
		 * @return whether any values have changed.
		 */
		bool copyFrom( const Function & function );
		/**
		 * \return the function ID, used to identify it from the parser.
		 */
		uint id() const { return m_id; }
		/**
		 * \see id()
		 */
		void setId( uint id ) { m_id = id; }
		QVector<Equation*> eq;
		/**
		 * \return A reference to the appearance of the given plot type.
		 */
		PlotAppearance & plotAppearance( PMode plot );
		/**
		 * \return The appearance of the given plot type.
		 */
		PlotAppearance plotAppearance( PMode plot ) const;
		/**
		 * \returns true if all plots are hidden (i.e. plotAppearance().visible
		 * is false for all plot types).
		 * \returns false otherwise.
		 */
		bool allPlotsAreHidden() const;
		/**
		 * Custom plot range, lower boundary.
		 */
		Value dmin;
		/**
		 * Custom plot range, upper boundary.
		 */
		Value dmax;
		
		ParameterSettings m_parameters;
		
		bool usecustomxmin:1;
		bool usecustomxmax:1;
		// TODO double slider_min, slider_max; ///< extreme values of the slider
		
		/**
		 * For use with implicit functions, when either x or y is held fixed.
		 */
		enum ImplicitMode
		{
			FixedX,
			FixedY,
			UnfixedXY
		};
		ImplicitMode m_implicitMode;
		/**
		 * The value of x when this is an implicit function and x is fixed.
		 */
		double x;
		/**
		 * The value of y when this is an implicit function and y is fixed.
		 */
		double y;
		
		/**
		 * A list with all functions that this function depends on.
		 */
		QList<int> m_dependencies;
		
	protected:
		uint m_id;
		const Type m_type;
		
		PlotAppearance f0;		///< The actual function - the "zero'th derivative"
		PlotAppearance f1;		///< First derivative
		PlotAppearance f2;		///< Second derivative
		PlotAppearance integral;	///< integral
};




/**
 * Uniquely identifies a single plot (i.e. a single curvy line in the View).
 */
class Plot
{
	public:
		Plot();
		
		bool operator == ( const Plot & other ) const;
		
		void setFunctionID( int id );
		/**
		 * Changes the plotMode equivalent to differentiating.
		 */
		void differentiate();
		/**
		 * Changes the plotMode equivalent to integrating.
		 */
		void integrate();
		int functionID() const { return m_functionID; }
		/**
		 * \return a pointer to the function with ID as set by setFunctionID
		 */
		Function * function() const { return m_function; }
		/**
		 * The color that the plot should be drawn with.
		 */
		QColor color() const;
		/**
		 * Parameter in use.
		 */
		Parameter parameter;
		/**
		 * Which derivative.
		 */
		Function::PMode plotMode;
		/**
		 * Converts the plotMode to the derivative number, e.g.
		 * Function::Derivative1 -> 1, and Function::Integral -> -1
		 */
		int derivativeNumber() const;
		/**
		 * Assigned when Function::allPlots() is called. The plots for each
		 * plotMode are numbered 0 to *.
		 */
		int plotNumber;
		/**
		 * The total number of plots of the same plotMode as this.
		 */
		int plotNumberCount;
		/**
		 * Updates the current working parameter value in the function that
		 * this plot is for and the plus-minus signature for the function's
		 * equations.
		 */
		void updateFunction() const;
		/**
		 * For differential equations, which state to draw.
		 */
		int state;
		/**
		 * For equations containing a plus-minus symbols, this indicates
		 * whether to take the plus or the minus for each one. The list is for
		 * each equation of the function (so typically, the list will only be
		 * of size one, but parametric functions will have two).
		 */
		QList< QVector<bool> > pmSignature;
		
	protected:
		void updateCached();
		
		int m_functionID;			///< ID of function
		Function * m_function;		///< Cached pointer to function
};


#endif	// FUNCTION_H
