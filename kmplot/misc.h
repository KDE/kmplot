#ifndef misc_included
#define misc_included

#include "xparser.h"
#include "diagr.h"
#include <kapp.h>
#include <kconfig.h>
#include <qstring.h>
#include <qcolor.h>

#define	KP_VERSION	"0.4"


extern KConfig *kc;
extern XParser	ps;

extern	char mode,			// Diagrammodus
	     g_mode;			// Rastermodus

extern	int koordx,			// 0 => [-8|+8]
	    koordy,				// 1 => [-5|+5]
							// 2 => [0|+16]
							// 3 => [0|+10]
	    AchsenDicke,
	    GitterDicke,
	    TeilstrichDicke,
	    TeilstrichLaenge;

extern	double xmin,        // min. x-Wert
	       	   xmax,        // max. x-Wert
               ymin,        // min. y-Wert
               ymax,        // max. y-Wert
               sw,			// Schrittweite
               rsw,
               tlgx,		// x-Achsenteilung
               tlgy,		// y-Achsenteilung
               drskalx,		// x-Ausdruckskalierung
               drskaly;		// y-Ausdruckskalierung

extern QString datei,		// Dateiname
			xminstr,		// String f�r xmind
            xmaxstr,		// String f�r xmaxd
    		yminstr,		// String f�r ymind
    		ymaxstr,		// String f�r ymaxd
    		tlgxstr,        // String f�r tlgx
    		tlgystr,        // String f�r tlgy
    		drskalxstr,     // String f�r drskalx
    		drskalystr;     // String f�r drskaly

extern	QRgb AchsenFarbe,
	     GitterFarbe;


void 	init();				// Initialisierung


//void getdefaults();
//void putdefaults();

#endif	// misc_included
