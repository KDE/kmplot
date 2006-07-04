/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
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

/** @file parser.h
 * \brief Contains the parser core class Parser. */

#ifndef parser_included
#define parser_included
 
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>

#include "function.h"
#include "vector.h"

class Parser;

// Voreinstellungen bei Verwendung des Standardkonstruktors :

#define	MEMSIZE		500	///< memory size for tokens
#define	STACKSIZE	50	///< stack depth

//@{
/** Token type. */
enum Token
{
	KONST,		// double value follows
	VAR,		// get a parameter (e.g. x or k)
	PUSH,		// push value to stack
	PLUS,		// add
	MINUS,		// subtract
	PM,			// plus-minus; add or subtract depending on the current signature
	MULT,		// multiply
	DIV,		// divide
	POW,		// exponentiate
	NEG,		// negate
	FKT_1,		// address to function with 1 argument follows
	FKT_N,		// address to functions with an indefinite number of arguments follows
	UFKT,		// address to user defined function follows
	ENDE,		// end of function
	SQRT		// take square root
};


const int legendreCount = 7; // number of legendre polynomials we allow for
const int ScalarCount = 41+legendreCount;	// number of mathematical scalar functions
const int VectorCount = 3; // number of vector functions
//@}

//@{
/** Predefined mathematical function with one variable. */
double ln(double x);
double llog(double x);
double sign(double x);
double heaviside(double x);
double sqr(double x);
double arsinh(double x);
double arcosh(double x);
double artanh(double x);

double sec(double x);
double cosec(double x);
double cot(double x);
double arcsec(double x);
double arccosec(double x);
double arccot(double x);

double sech(double x);
double cosech(double x);
double coth(double x);
double arsech(double x);
double arcosech(double x);
double arcoth(double x);

double lcos(double x);
double lsin(double x);
double ltan(double x);

double lcosh(double x);
double lsinh(double x);
double ltanh(double x);

double arccos(double x);
double arcsin(double x);
double arctan(double x);

double legendre0(double x);
double legendre1(double x);
double legendre2(double x);
double legendre3(double x);
double legendre4(double x);
double legendre5(double x);
double legendre6(double x);

/** Predefined mathematical functions with an indefinite number of variables. */
double min( const Vector & x );
double max( const Vector & x );
double mod( const Vector & x );


struct ScalarFunction
{
	QString name;
	double (*mfadr)(double);
};


struct VectorFunction
{
	QString name;
	double (*mfadr)(const Vector &);
};


class Constant
{
	public:
		Constant( QChar c='A', double v=0)
		{
			constant = c;
			value = v;
		};
	
		QChar constant;
		double value;
};


/**
 * Fixes user-entered expressions into a form that can be handled by the
 * parser. Also keeps track of how the string was modified, so that if an error
 * occurs, then the correct position can be reported to the user.
 * \note The convention used here is that the first letter in a string
 * is at position zero.
 */
class ExpressionSanitizer
{
	public:
		ExpressionSanitizer( Parser * parent );
		
		/**
		 * Lots of changes to make it happy for the parser (e.g. adding extra
		 * *-characters, remove spaces, replace the locale .-character with '.',
		 * etc). This function will initialize m_evalMap.
		 * \param str The string to be fixed.
		 */
		void fixExpression( QString * str );
		/**
		 * \return the position in the input string (as given to fixExpression)
		 * that corresponds to the outputted string.
		 */
		int realPos( int evalPos );
		
	protected:
		/**
		 * Maps the position of the string returned by fixExpression to that
		 * passed to it. This is so that if the parser comes across an error in the
		 * sanitized expression, this gives the corresponding position in the user
		 * string.
		 */
		QVector<int> m_map;
		
		void remove( const QString & str );
		void remove( const QChar & str );
		void replace( QChar before, QChar after );
		void replace( QChar before, const QString & after );
		void replace( int pos, int len, const QString & after );
		void insert( int i, QChar ch );
		void append( QChar str );
		void stripWhiteSpace();
		
		/**
		 * Prints the map and str to stdout; for debugging purposes.
		 */
		void displayMap();
		
		QString * m_str;
		
		QString m_decimalSymbol;
		Parser * m_parser;
};


/**
 * @short Manages a list of constants.
 */
class Constants
{
	public:
		Constants( Parser * parser );
		
		/// Loading the constants at the start
		void load();
		/// Loading the constants when closing the program
		void save();
		/// @return if the constant name is valid
		bool isValidName( QChar name );
		/// @returns an iterator to the constant with the given name, or constants::end if not found
		QVector<Constant>::iterator find( QChar name );
		/// removes the constant with the given name from the constants list
		void remove( QChar name );
		/// adds the constant to the internal list (overwriting any previous constant with the same name)
		void add( Constant c );
		/// @return whether the constant with the given name exists
		bool have( QChar name ) const;
		/// @return a unique (i.e. unused) constant name
		QChar generateUniqueName();
		/// @return a copy of the list of constants
		QVector<Constant> all() const { return m_constants; }
		
	protected:
		QVector<Constant> m_constants;
		Parser * m_parser;
};


/** @short Parser.
 *
 * Tokenizes a function equation to be evaluated.
 */
class Parser : public QObject
{
	Q_OBJECT
	public:
	
		enum Error
		{
			ParseSuccess,
			SyntaxError,
			MissingBracket,
			UnknownFunction,
			InvalidFunctionVariable,
			TooManyFunctions,
			MemoryOverflow,
			StackOverflow,
			FunctionNameReused , ///< function name already used
			RecursiveFunctionCall,
			EmptyFunction,
			CapitalInFunctionName, ///< function name contains a capital letter
			NoSuchFunction,
			UserDefinedConstantInExpression, ///< evalation expression may not use user defined constants
			ZeroOrder, ///< zero-order differential
			TooManyPM, ///< too many plus-minus symbols
			InvalidPM, ///< Not allowed to have a plus-minus symbol, e.g. in a constant expression
			TooManyArguments ///< Too many arguments in a function, e.g. "f(x,a,b,c)"
		};
	
		~Parser();
	
		/**
		 * \return the list of predefined function names.
		 */
		QStringList predefinedFunctions() const;
		/**
		 * @return A string that is safe to use as a number in a string to be
		 * parsed. This is needed as e.g. "1.2e-3" is not allowed (e is a
		 * constant) - so cannot use the QString::number.
		 */
		static QString number( double value );
		/**
		 * Calls the array version of this function, after inserting the value
		 * of the equation's parameter into x.
		 */
		double fkt( Equation * it, double x );
		double fkt( uint id, int eq, double x );
	
		/**
		 * Evaluates the given expression \p str.
		 */
		double eval( const QString & str );
		/**
		 * Adds a user defined function with the given equation. The new
		 * function's ID-number is returned.
		 */
		int addFunction( const QString & str1, const QString & str2, Function::Type type );
		/**
		 * Removes the function with the given id.
		 */
		bool removeFunction( uint id );
		bool removeFunction( Function *item);
		/**
		 * Returns the ID-number of the function "name". If the function
		 * couldn't be found, -1 is returned.
		 */
		int fnameToID(const QString &name);
		/**
		 * \return An error string appropriate for the last error. If there was
		 * no error, then this will just return an empty string.
		 */
		QString errorString() const;
		/**
		 * Position where the error occurred.
		 */
		int errorPosition() const { return m_errorPosition; }
		/**
		 * If showMessageBox is true, an error message box will appear if an
		 * error was found.
		 * \return The current error value.
		 */
		Error parserError( bool showMessageBox );
		/**
		 * Called by e.g. Equation::setFstr before the parser stage is reached.
		 */
		void setParserError( Error error ) { m_error = error; }
		/**
		 * \return the number of radians per angle-unit that the user has
		 * selected (i.e. this will return 1.0 if the user has selected
		 * radians; and PI/180 if the user has selected degrees).
		 */
		static double radiansPerAngleUnit();
		
		enum AngleMode { Radians = 0, Degrees = 1 };
		/**
		 * Sets the angle mode (in which the calculations are performed).
		 */
		void setAngleMode( AngleMode mode );
		/**
		* Initializes the function for evaluation. Called after the functions
		* fstr is set.
		*/
		void initEquation( Equation * equation );
		
		uint getNewId(); /// Returns the next ID-number
		uint countFunctions(); /// Returns how many functions there are
	
		/// The constants used by the parser
		Constants * constants() const { return m_constants; }
		
		/// @return the function with the given id
		Function * functionWithID( int id ) const;
		
		/// Points to the array of user defined functions, index by their IDs.
		QMap<int, Function *> m_ufkt;
	
	signals:
		/// emitted when a function is deleted
		void functionRemoved( int id );
		/// emitted when a function is added
		void functionAdded( int id );

	private:
		/**
		 * Returns the result of a calculation. \p x are parameters for the
		 * function (which are not necessarily all used).
		 */
		double fkt( Equation * it, const Vector & x );
		/** Mathematical function. */
		static ScalarFunction scalarFunctions[ScalarCount];
		static VectorFunction vectorFunctions[VectorCount];
	
		Error m_error;
		/// Position where the error occurred.
		int m_errorPosition;
	
		void heir1();
		void heir2();
		void heir3();
		void heir4();
		void primary();
		void addToken( Token token );
		void addConstant(double);
		void adduint(uint);
		void addfptr(double(*)(double));
		void addfptr( double(*)(const Vector &), int argCount );
		/**
		 * \p id Id of the function
		 * \p eq_id Which equation of the function to use
		 * \p args The number of variables being passed to the function
		 */
		void addfptr( uint id, uint eq_id, uint args );
		/**
		 * Attempts to \p string to the current evaluation text. If the text at
		 * the current evaluation position is matched, then the evaluation
		 * position is incremented past the length of the string and true is
		 * returned. Else the evaluation position remains unchanged, and false
		 * is returned.
		 */
		bool match( const QString & string );
		/**
		 * Continues to read the expression inside a brackets of a vector
		 * function until get to the end of the argument list.
		 * \return the number of arguments
		 */
		int readFunctionArguments();
        
		unsigned char
			*mem, 			    // Zeiger auf Speicher fr Token
			*mptr;			    // Zeiger fr Token
		double * m_stack;
		QString m_eval;
		int m_evalPos;
		int m_nextFunctionID;
		/// @return the m_eval starting at m_evalPos
		QString evalRemaining() const;
		Equation * m_currentEquation; // Pointer to the current function
		Equation * m_ownEquation; ///< used for parsing constants, etc, and ensures that m_currentEquation is never null
		static double m_radiansPerAngleUnit;
		Constants * m_constants;
		ExpressionSanitizer m_sanitizer;
		int m_pmAt; ///< When parsing an expression, which plus-minus symbol at
	
	private:
		friend class XParser;
		friend class ExpressionSanitizer;
		Parser();
};

#endif	// parser_included
