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
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*/

//	Die Funktion parse tokenisiert den als String übergebenen
//	Funktionsterm im Speicherbereich ab mem.
//
//						KDM  2.5.95


#ifndef parser_included
#define parser_included

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <klocale.h>
#include <kmessagebox.h>


// Voreinstellungen bei Verwendung des Standardkonstruktors :

#define	UFANZ		10	// max. Anzahl benutzerdefinierter Funktionen
#define	MEMSIZE		200	// Speichergröße für Token
#define	STACKSIZE	50	// Stacktiefe


//	Token-Liste :

#define KONST	0       // es folgt ein double-Wert
#define	XWERT	1       // x-Wert holen
#define KWERT   2		// Funktionsparameter holen
#define	PUSH	3       // Wert auf Stack
#define	PLUS	4       // Addition
#define	MINUS	5       // Subtraktion
#define	MULT	6       // Multiplikation
#define	DIV		7       // Division
#define	POW		8       // Potenzieren
#define NEG		9		// Negieren
#define FKT		10		// es folgt eine Funktionsadresse
#define	UFKT	11      // es folgt eine Adresse auf eine
// benutzerdefinierte Funktion
#define	ENDE	12      // Funktionsende

#define	FANZ	31		// Anzahl der math. Funktionen
// in mfkttab[]


double sign(double x),
sqr(double x),
arsinh(double x),
arcosh(double x),
artanh(double x);

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

class Parser
{
public:

	Parser();
	Parser(int, int, int);

	double	eval(QString);
	double	fkt(QString, double);
	double	fkt(int ix, double x) {return ufkt[ix].fkt(x);}
	int	addfkt(QString),
	delfkt(QString);

	int delfkt(int);
	int	chkfix(int),
	getfkt(int, QString&, QString&),
	getfix(QString);
	int getNextIndex();
	int errmsg();

	void setparameter(int ix, double k) {ufkt[ix].k=k;}

	~Parser();

	int err,	// Fehlercodes:
			// 0 => parse erfolgreich
			// 1 => Syntaxfehler
			// 2 => fehlende Klammer
			// 3 => Funktion nicht bekannt
			// 4 => ungültige Funktionsvariable
			// 5 => zu viele Funktionen
			// 6 => Speicherüberlauf
			// 7 => Stacküberlauf
			// 8 => Funktionsname bereits vergeben
			//  9 => rekursiver Funktionsaufruf
			
	errpos, 	// Fehlerposition
	ufanz;		// max. Anzahl benutzer-
	                // definierter Funktionen

protected:

    class Ufkt
	{
	public:

		Ufkt();

		double	fkt(double);	// benutzerdefinierte Funktion

		~Ufkt();

		unsigned
		char *mem, 		        // Zeiger auf Tokenspeicher
		*mptr;		            // Zeiger auf Token
		QString	fname, 	        // Funktionsname
		fvar, 		            // Funktionsvariable
		fpar, 		            // Parameter
		fstr;		            // Funktionsterm
		int	memsize, 	        // Größe des Tokenspeichers
		stacksize;	            // Größe des Stack
		double	k;		        // Funktionsparameter

	}
	*ufkt;

	struct Mfkt
	{   const char *mfstr;
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
	*mem, 			    // Zeiger auf Speicher für Token
	*mptr;			    // Zeiger für Token
	const
	char *lptr;			// Zeiger für Funktions-String
	int	memsize,        // Größe des Tokenspeichers
	stacksize,          // Größe des Stack
	ixa;			    // Index der aktuellen Funktion
	double *stack, 		// Zeiger auf Stackanfang
	*stkptr;		    // Stackpointer
};

#endif	// parser_included
