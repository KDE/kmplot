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
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*/

/** @file parser.h
 * \brief Contains the parser core class Parser. */
 
// Qt includes
#include <qstring.h>
//#include <qvaluelist.h>
#include <qvaluevector.h>


#ifndef parser_included
#define parser_included

// Voreinstellungen bei Verwendung des Standardkonstruktors :

#define	MEMSIZE		200	///< memory size for tokens
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
#define	FANZ	31	// number of mathematical functions in mfkttab[]
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

//@}

class Constant
{
public:
	Constant( char c='A', double v=0)
	{
	constant = c;
	value = v;
	};
	
	char constant;
	double value;
};

/** @short Parser.
 *
 * Tokenizes a function equation to be evaluated.
 */
class Parser
{
public:
	Parser(int, int);
	~Parser();
        
	/// Evaluates the given expression.
	double eval(QString);
	/// Adds a user defined function with the given equation.
	int addfkt(QString);
	/// Removes the function with the given index.
	bool delfkt(int);
	/// Returns the index of the function with the given name.
	bool getfix(QString);
	/// Shows an error message box.
	int errmsg();
	/// ?
	//void setParameter(int ix, double k) {ufkt[ix].k=k;}
	/// return the angletype
	static double anglemode();
	/// sets the angletype. TRUE is radians and FALSE degrees
	void setAngleMode(int);
	/// sets the decimal symbol
	void setDecimalSymbol(const QString );
	/// Reparse the function. It also do a grammer check for the expression which you set before calling the function with this.ufkt[ix].fstr
	void reparse(int ix);
        
        uint getNewId();
        
        int idValue(int const ix);
        int ixValue(uint const id);

	QValueVector<Constant> constant;
	
	/// Error codes.
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
	 */
	int err,	
	errpos; 	///< Position where the error occurred.


        /** User function. */
	class Ufkt
	{
	public:
		Ufkt();
		~Ufkt();
                void setParameter(double const &p) {k = p; };
                
                uint id;
		unsigned char *mem;	///< Pointer to the allocated memory for the tokens.
		unsigned char *mptr;	///< Pointer to the token.
		QString	fname; 	        ///< Name of the function.
		QString fvar;		///< Dummy variable.
		QString fpar;		///< Parameter.
		QString fstr;		///< Function expression.
		int memsize;	 	///< Size of token memory
		int stacksize;		///< Size of the stack.
		double k,		///< Function parameter.
		oldy;			///< The last y-value needed for Euler's method
        };
        QValueVector<Ufkt> ufkt;///< Points to the array of user defined functions.
        
        /// Removes the function
        void delfkt(QValueVector<Parser::Ufkt>::iterator );
        
        double fkt(Ufkt *it, double const x);
        double fkt(uint i, double const x);
        
        void reparse(Ufkt *item);
			
protected:
	/** Mathematical function. */
	struct Mfkt
	{
		const char *mfstr;
		double (*mfadr)(double);
	};
	static Mfkt mfkttab[FANZ];

private:
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

        int  const   memsize,        // Gr�e des Tokenspeichers
        stacksize;// Gr�e des Stack
        
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
