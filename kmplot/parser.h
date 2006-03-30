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

/** @file parser.h
 * \brief Contains the parser core class Parser. */
 
 
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>

#include "parseriface.h"

#ifndef parser_included
#define parser_included

class Parser;

// Voreinstellungen bei Verwendung des Standardkonstruktors :

#define	MEMSIZE		500	///< memory size for tokens
#define	STACKSIZE	50	///< stack depth

//@{
/** Token type. */
#define KONST	0       // double value follows
#define	XWERT	1       // get x value
#define KWERT   2	// get function parameter
#define	PUSH	3       // push value to stack
#define	PLUS	4       // add
#define	MINUS	5       // subtract
#define	MULT	6       // multiply
#define	DIV	7       // divide
#define	POW	8       // exponentiate
#define NEG	9	// negate
#define FKT	10	// address to function followes
#define	UFKT	11      // address to user defined function follows
#define	ENDE	12      // end of function
#define	YWERT	13       // get y value

#define	FANZ	35	// number of mathematical functions in mfkttab[]
//@}

//@{
/** Predefined mathematical function. */
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
        
		bool f_mode:1; ///< \a f_mode == 1: draw the plot.
		bool f1_mode:1; ///< \a f1_mode == 1.  draw the 1st derivative, too.
		bool f2_mode:1; ///< \a f2_mode == 1.  draw the 2nd derivative, too.
		bool integral_mode:1; ///< \a f2_mode == 1.  draw the integral, too.
		bool integral_use_precision:1; ///< The user can specify an unic precision for numeric prime-functions
		double linewidth,f1_linewidth,f2_linewidth, integral_linewidth; ///< Line width in mm.
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
		QColor color, ///< current color.
		f1_color, f2_color, integral_color;
		int use_slider; ///< -1: none (use list), else: slider number
		QList<ParameterValueItem> parameters; ///< List with parameter for the function
		bool usecustomxmin:1;
		bool usecustomxmax:1;
        	// TODO double slider_min, slider_max; ///< extreme values of the slider
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
 * @short Mangages a list of constants.
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
		bool have( QChar name );
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
class Parser : public QObject, virtual public ParserIface
{
	Q_OBJECT
public:
	
	enum Error
	{
		ParseSuccess = 0,
		SyntaxError = 1,
		MissingBracket = 2,
		UnknownFunction = 3,
		InvalidFunctionVariable = 4,
		TooManyFunctions = 5,
		MemoryOverflow = 6,
		StackOverflow = 7,
		FunctionNameReused = 8, ///< function name already used
		RecursiveFunctionCall = 9,
		NoSuchConstant = 10,
		EmptyFunction = 11,
		CapitalInFunctionName = 12, ///< function name contains a capital letter
		NoSuchFunction = 13,
		UserDefinedConstantInExpression = 14, ///< evalation expression may not use user definded constants
	};
	
	Parser();
	~Parser();
	
	/// Returns the result of a calculation
	double fkt(Ufkt *it, double const x);
	double fkt(uint id, double const x);
	
	/// Evaluates the given expression.
	double eval(QString);
	/// Adds a user defined function with the given equation. The new function's ID-number is returned.
	virtual int addfkt(QString);
	/// Removes the function with the given id.
	bool delfkt(uint id);
	bool delfkt( Ufkt *item);
	
	/// Returns the ID-number of the function "name". If the function couldn't be found, -1 is returned.
	int fnameToId(const QString &name);
	/// Returns the current error value. If showMessageBox is true, an error message box will appear if an error was found
	Error parserError( bool showMessageBox );
	
	/// return the angletype
	static double anglemode();
	/// Sets the angletype. TRUE is radians and FALSE degrees
	void setAngleMode(int);
	/// sets the decimal symbol
	void setDecimalSymbol(const QString );
	
	/// reparse the function. It also do a grammer check for the expression
	void reparse(int id);
	void reparse(Ufkt *item);
	
	uint getNewId(); /// Returns the next ID-number
	uint countFunctions(); /// Returns how many functions there are

	/// The constants used by the parser
	Constants * constants() const { return m_constants; }
	
	/// @return the function with the given id
	Ufkt * functionWithID( int id ) const;
	
	/// Points to the array of user defined functions, index by their IDs.
	QMap<int, Ufkt *> m_ufkt;
	
signals:
	/// emitted when a function is deleted
	void functionRemoved( int id );
	/// emitted when a function is added
	void functionAdded( int id );

private:
	/** Mathematical function. */
	struct Mfkt
	{
		const char *mfstr;
		double (*mfadr)(double);
	};
	static Mfkt mfkttab[FANZ];
	
	Error err;
	///  Position where the error occurred.
	int errpos;

	void fix_expression(QString &, int const); ///adding extra *-characters, remove spaces and replace the locale .-character with '.'
	
	void heir1();
	void heir2();
	void heir3();
	void heir4();
	void primary();
	void addtoken(unsigned char);
	void addwert(double);
	void addfptr(double(*)(double));
	void addfptr(uint );
	int match( const QString & );
        
	unsigned
	char evalflg, 		// 0 => String wird tokenisiert
	                    // 1 => String wird direkt ausgewertet
	*mem, 			    // Zeiger auf Speicher fr Token
	*mptr;			    // Zeiger fr Token
	QString m_eval;
	int m_evalPos;
	int m_nextFunctionID;
	/// @return the m_eval starting at m_evalPos
	QString evalRemaining() const;
	Ufkt * current_item; // Pointer to the current function
	Ufkt * m_ownFunction; ///< used for parsing constants, etc, and ensures that current_item is never null
	double *stack, 		// Zeiger auf Stackanfang
	*stkptr;		    // Stackpointer
	static double  m_anglemode;
	QString m_decimalsymbol;
	Constants * m_constants;
};

#endif	// parser_included
