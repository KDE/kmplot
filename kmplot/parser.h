/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.
*
*/

/** @file parser.h
 * \brief Contains the parser core class Parser. */
 
// Qt includes
#include <qstring.h>
#include <qvaluevector.h>

#include "parseriface.h"

#ifndef parser_included
#define parser_included

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
#define	FANZ	34	// number of mathematical functions in mfkttab[]
//@}

//@{
/** Predefined mathematical function. */
double ln(double x);
double llog(double x);
double sign(double x);
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
		}
		ParameterValueItem() {;}
		QString expression;
		double value;
};


/** Here are all atitrbutes for a function stored. */
class Ufkt
{
	public:
		Ufkt();
		~Ufkt();
		/// Sets the parameter 
		void setParameter(double const &p) {k = p; }
        
		uint id;
		unsigned char *mem;     ///< Pointer to the allocated memory for the tokens.
		unsigned char *mptr;    ///< Pointer to the token.
		QString fname;          ///< Name of the function.
		QString fvar;           ///< Dummy variable.
		QString fpar;           ///< Parameter.
		QString fstr;           ///< Function expression.
		double k,               ///< Function parameter.
		oldy;                   ///< The last y-value needed for Euler's method
		QValueList<int> dep;   /// A list with all functions this function depends on
        
		bool f_mode, ///< \a f_mode == 1: draw the plot.
		f1_mode, ///< \a f1_mode == 1.  draw the 1st derivative, too.
		f2_mode,///< \a f2_mode == 1.  draw the 2nd derivative, too.
		integral_mode, ///< \a f2_mode == 1.  draw the integral, too.
		integral_use_precision; ///< The user can specify an unic precision for numeric prime-functions
		int linewidth,f1_linewidth,f2_linewidth, integral_linewidth; ///< Line width.
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
		QRgb color, ///< current color.
		f1_color, f2_color, integral_color;
		int use_slider; ///< -1: none (use list), else: slider number
		QValueList<ParameterValueItem> parameters; ///< List with parameter for the function
		bool usecustomxmin, usecustomxmax;
        	// TODO double slider_min, slider_max; ///< extreme values of the slider
};

class Constant
{
public:
	Constant( char c='A', double v=0)
	{
	constant = c;
	value = v;
	}
	
	char constant;
	double value;
};

/** @short Parser.
 *
 * Tokenizes a function equation to be evaluated.
 */
class Parser : virtual public ParserIface
{
public:
	Parser();
	~Parser();
	
	/**
       * @return A string that is safe to use as a number in a string to be
       * parsed. This is needed as e.g. "1.2e-3" is not allowed (e is a
       * constant) - so cannot use the QString::number.
       */
      static QString number( double value );
	
        /// Returns the result of a calculation
        double fkt(Ufkt *it, double const x);
        double fkt(uint id, double const x);
	
	/// Evaluates the given expression.
	double eval(QString);
	/// Adds a user defined function with the given equation. The new function's ID-number is returned.
	int addfkt(QString);
        /// Removes the function with the given id.
	bool delfkt(uint id);
        bool delfkt( Ufkt *item);
	
	/// Returns the ID-number of the function "name". If the function couldn't be found, -1 is returned.
	int fnameToId(const QString &name);
	/// Returns the current error value. If showMessageBox is true, an error message box will appear if an error was found
	int parserError(bool showMessageBox=TRUE);
	
	/// return the angletype
	static double anglemode();
	/// Sets the angletype. TRUE is radians and FALSE degrees
	void setAngleMode(int);
	/// sets the decimal symbol
	void setDecimalSymbol(const QString );
	
        /// reparse the function. It also do a grammer check for the expression
	void reparse(int ix);
        void reparse(Ufkt *item);
	
        uint getNewId(); /// Returns the next ID-number
        int idValue(int const ix); /// Converts an index-value to an ID-number
        int ixValue(uint const id);/// Converts an ID-numer to an index-value
        uint countFunctions(); /// Returns how many functions there are

	QValueVector<Constant> constant;
        QValueVector<Ufkt> ufkt;///< Points to the array of user defined functions.

private:
	/** Mathematical function. */
	struct Mfkt
	{
		const char *mfstr;
		double (*mfadr)(double);
	};
	static Mfkt mfkttab[FANZ];
	
	/// Error codes:
        /**
         * The values have following meanings:
	 * \li  0 => parse success
	 * \li  1 => syntax error
	 * \li  2 => missing bracket
	 * \li  3 => function unknown
	 * \li  4 => function variable not valid
	 * \li  5 => too much functions
	 * \li  6 => memory overflow
	 * \li  7 => stack overflow
	 * \li  8 => function name already used
	 * \li  9 => recursive function call
	 * \li  10 => didn't found the wanted constant
	 * \li   11 => emtpy function
	 * \li   12 => function name contains a capital letter
	 * \li   13 => function could not be found
	 * \li   14 => evalation expression may not use user definded constants
	 */
	int err;
	///  Position where the error occurred.
        int errpos;

        void fix_expression(QString &, int const); ///adding extra *-characters, remove spaces and replace the locale .-character with '.'
        
	void ps_init(),
	heir1(),
	heir2(),
	heir3(),
	heir4(),
	primary(),
	addtoken(unsigned char),
	addwert(double),
	addfptr(double(*)(double)),
	addfptr(uint );
	int match(const char*);
        
	unsigned
	char evalflg, 		// 0 => String wird tokenisiert
	                    // 1 => String wird direkt ausgewertet
	*mem, 			    // Zeiger auf Speicher fr Token
	*mptr;			    // Zeiger fr Token
	const
	char *lptr;			// Zeiger fr Funktions-String
        Ufkt *current_item; // Pointer to the current function
	double *stack, 		// Zeiger auf Stackanfang
	*stkptr;		    // Stackpointer
	static double  m_anglemode;
	QString m_decimalsymbol;
};

#endif	// parser_included
