#ifndef xparser_included
#define xparser_included

#include <parser.h>
#include <qcolor.h>


class XParser : public Parser
{	public:
	
	XParser();
	XParser(int anz, int m_size, int s_size);
	
	int getext(int);
	int delfkt(int);
	double a1fkt(int, double, double h=1e-3),
	       a2fkt(int, double, double h=1e-3);
	
	~XParser();
	
	int dicke0;				// Linienstärke Voreinstellung
	  
	struct FktExt           // Funktionsattribute:
	{	char f_mode, 		// 1 => Graph von f zeichnen
		     f1_mode,		// 1 => Graph von f' zeichnen
		     f2_mode;		// 1 => Graph von f" zeichnen
		int  dicke, 		// Stiftdicke
		     k_anz;  		// Länge der Parameterliste
		double dmin,    	// Definitionsbereich
		       dmax,
		       k_liste[10];	// Parameterliste
		       
		QString extstr;     // vollständiger Eingabeterm
		QRgb farbe,         // Farbe des Graphen
		     farbe0;		// Farbvoreinstellung
	} *fktext;
};

#endif //xparser_included

