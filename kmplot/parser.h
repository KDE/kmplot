//	Die Funktion parse tokenisiert den als String �bergebenen
//	Funktionsterm im Speicherbereich ab mem.
//
//						KDM  2.5.95 

#ifndef parser_included
#define parser_included

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <qstring.h>
#include <klocale.h>
#include "kmessagebox.h"


// Voreinstellungen bei Verwendung des Standardkonstruktors :

#define	UFANZ		10	// max. Anzahl benutzerdefinierter Funktionen
#define	MEMSIZE		200	// Speichergr��e f�r Token 
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

#define	FANZ	17		// Anzahl der math. Funktionen 
						// in mfkttab[]


double	sqr(double x),
	arsinh(double x),
	arcosh(double x),
	artanh(double x);
      
	
class Parser  
{	public:

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
		getfix(QString),
		errmsg();
		
	void	setparameter(int ix, double k) {ufkt[ix].k=k;}
		
	~Parser();
	
	int	err,		// Fehlercode:
					// 	0 => parse erfolgreich
					//	1 => Syntaxfehler
					//	2 => fehlende Klammer
					//	3 => Funktion nicht bekannt
					//	4 => ung�ltige Funktionsvariable
					//	5 => zu viele Funktionen
					//	6 => Speicher�berlauf
					//	7 => Stack�berlauf
                    //  8 => Funktionsname bereits vergeben
		errpos,		// Fehlerposition
	    ufanz;		// max. Anzahl benutzer-
					// definierter Funktionen		
			
	protected:
	class Ufkt
	{	public:

		Ufkt();
	
		double	fkt(double);	// benutzerdefinierte Funktion
	
		~Ufkt();
	
		unsigned
		char *mem,		// Zeiger auf Tokenspeicher
			 *mptr;		// Zeiger auf Token
		QString	fname,	// Funktionsname
			fvar,		// Funktionsvariable
			fpar,		// Parameter
			fstr;		// Funktionsterm
        int	memsize,	// Gr��e des Tokenspeichers
   			stacksize;	// Gr��e des Stack
   		double	k;		// Funktionsparameter
   				
	} *ufkt;

	struct Mfkt
	{	const char 	*mfstr;
		double 	(*mfadr)(double);
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
	int  match(const char*);	
    
	unsigned
	char evalflg,		// 0 => String wird tokenisiert
						// 1 => String wird direkt ausgewertet
		*mem,			// Zeiger auf Speicher f�r Token
		*mptr;			// Zeiger f�r Token
	const
	char *lptr;			// Zeiger f�r Funktions-String
   	int	memsize,        // Gr��e des Tokenspeichers
   		stacksize,      // Gr��e des Stack
   		ixa;			// Index der aktuellen Funktion
	double *stack,		// Zeiger auf Stackanfang
		   *stkptr;		// Stackpointer
};

#endif	// parser_included
