#ifndef misc_included
#define misc_included

#include "xparser.h"
#include "diagr.h"
#include <kapplication.h>
#include <kconfig.h>
#include <qstring.h>
#include <qcolor.h>

#define	KP_VERSION	"0.4"


extern KConfig *kc;
extern XParser	ps;

extern	int mode,			// Diagrammodus
	     g_mode;			// Rastermodus

extern	int koordx,			// 0 => [-8|+8]
	    koordy,				// 1 => [-5|+5]
							// 2 => [0|+16]
							// 3 => [0|+10]
							// 4 => custom...
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
			xminstr,		// String für xmind
            xmaxstr,		// String für xmaxd
    		yminstr,		// String für ymind
    		ymaxstr,		// String für ymaxd
    		tlgxstr,        // String für tlgx
    		tlgystr,        // String für tlgy
    		drskalxstr,     // String für drskalx
    		drskalystr;     // String für drskaly

extern QString font_header, font_axes; // Font family names

extern	QRgb AchsenFarbe,
	     GitterFarbe;

extern bool printtable;		// header table printing option

void 	init();				// Initialisierung

/*
 * evaluates the predefined axes settings (kkordx/y)
 * @result true, if the evaluation of the strings was successful
 */
bool coordToMinMax( const int koord, double &min, double &max, const QString minStr, const QString maxStr );

//void getdefaults();
//void putdefaults();

#endif	// misc_included
