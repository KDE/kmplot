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

#ifndef parser_included
#define parser_included

// Voreinstellungen bei Verwendung des Standardkonstruktors :


#define	UFANZ		10	///< max. count of user defined functions
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
#define	FANZ	31	// number of mathematical functions in mfkttab[]
//@}

//@{
/** Predefined mathematical function. */
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
//@}

/** @short Parser.
 *
 * Tokenizes a function equation to be evaluated.
 */
class Parser
{
public:

	Parser();
	Parser(int, int, int);

	~Parser();
	
	/// Evaluates the given expression.
	double eval(QString);
	/// Evaluates the function with the given name at the position.
	double fkt(QString, double);
	/// Evaluates the function with the given index at the position.
	double fkt(int ix, double x) {return ufkt[ix].fkt(x);}
	/// Adds a user defined function with the given equation.
	int addfkt(QString);
	/// Removes the function with the given name.
	int delfkt(QString);
	/// Removes the function with the given index.
	int delfkt(int);
	/// Returns name and expression of the function with the given index.
	int getfkt(int, QString&, QString&);
	/// Checks, if at the given index a function is stored.
	int chkfix(int);
	/// Returns the index of the function with the given name.
	int getfix(QString);
	/// Returns the lowest index in the array of user defined functions which is empty, 
	/// or -1, if the array is full.
	int getNextIndex();
	/// Shows an error message box.
	int errmsg();
	/// ?
	void setparameter(int ix, double k) {ufkt[ix].k=k;}


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
	 * \li  8 => function name allready used
	 * \li  9 => recursive function call
	 */
	int err,	
	errpos, 	///< Position where the error occured.
	ufanz;		///< Max. count of user defined functions.

protected:

	/** User function. */
	class Ufkt
	{
	public:
		Ufkt();
		~Ufkt();
		double	fkt(double);	///< User defined function.

		unsigned char *mem;	///< Pointer to the allocated memory for the tokens.
		unsigned char *mptr;	///< Pointer to the token.
		QString	fname; 	        ///< Name of the function.
		QString fvar;		///< Dummy variable.
		QString fpar;		///< Parameter.
		QString fstr;		///< Function expression.
		int memsize;	 	///< Size of token memory
		int stacksize;		///< Size of the stack.
		double k;		///< Function parameter.

	}
	*ufkt; ///< Points to the array of user defined functions.

	/** Mathematical function. */
	struct Mfkt
	{
		const char *mfstr;
		double (*mfadr)(double);
	};
	static Mfkt mfkttab[FANZ];

private:

	void ps_init(int, int, int),
	heir1(),
	heir2(),
	heir3(),
	heir4(),
	primary(),
	addtoken(unsigned char),
	addwert(double),
	addfptr(double(*)(double)),
	addfptr(Ufkt*);
	int match(const char*);

	unsigned
	char evalflg, 		// 0 => String wird tokenisiert
	                    // 1 => String wird direkt ausgewertet
	*mem, 			    // Zeiger auf Speicher fr Token
	*mptr;			    // Zeiger fr Token
	const
	char *lptr;			// Zeiger fr Funktions-String
	int	memsize,        // Gr�e des Tokenspeichers
	stacksize,          // Gr�e des Stack
	ixa;			    // Index der aktuellen Funktion
	double *stack, 		// Zeiger auf Stackanfang
	*stkptr;		    // Stackpointer
};

#endif	// parser_included
