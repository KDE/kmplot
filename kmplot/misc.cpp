#include "misc.h"


KApplication *ka;
KConfig *kc;

XParser ps(10, 200, 20);

	char mode,			// Diagrammodus
	     g_mode;		// Rastermodus

	int koordx,			// 0 => [-8|+8]
	    koordy,			// 1 => [-5|+5]
						// 2 => [0|+16]
						// 3 => [0|+10]
	    AchsenDicke,
	    GitterDicke,
	    TeilstrichDicke,
	    TeilstrichLaenge;

	double xmin,                   	// min. x-Wert
	       xmax,                   	// max. x-Wert
           ymin,                	// min. y-Wert
           ymax,                	// max. y-Wert
             sw,					// Schrittweite
            rsw,					// rel. Schrittweite
           tlgx,					// x-Achsenteilung
           tlgy,					// y-Achsenteilung
           drskalx,					// x-Ausdruckskalierung
           drskaly;					// y-Ausdruckskalierung

    	QString	datei,				// Dateiname
    		xminstr,				// String für xmind
            xmaxstr,				// String für xmaxd
    		yminstr,				// String für ymind
    		ymaxstr,				// String für ymaxd
    		tlgxstr,                // String für tlgx
    		tlgystr,                // String für tlgy
    		drskalxstr,             // String für drskalx
    		drskalystr;             // String für drskaly

	QRgb AchsenFarbe,
	     GitterFarbe;



void init()
{	int ix;
	
        QColor tmp_color;
	mode=ACHSEN | PFEILE | EXTRAHMEN;
	koordx=koordy=0;
	rsw=1.;
	datei="";
	
	// Standardwerte für die Achsen
	
	kc->setGroup("Achsen");
	xminstr=kc->readEntry("xmin", "-8");
	xmin=ps.eval(xminstr); 
	xmaxstr=kc->readEntry("xmax", "8");
	xmax=ps.eval(xmaxstr);
	yminstr=kc->readEntry("ymin", "-8");
	ymin=ps.eval(yminstr);
	ymaxstr=kc->readEntry("ymax", "8");
	ymax=ps.eval(ymaxstr);

    tlgxstr=kc->readEntry("tlgx", "1");
	tlgx=ps.eval(tlgxstr);
    tlgystr=kc->readEntry("tlgy", "1");
	tlgy=ps.eval(tlgystr);

    drskalxstr=kc->readEntry("drskalx", "1");
	drskalx=ps.eval(drskalxstr);
    drskalystr=kc->readEntry("drskaly", "1");
	drskaly=ps.eval(drskalystr);

	AchsenDicke=kc->readNumEntry("Achsenstärke", 5);
	TeilstrichDicke=kc->readNumEntry("Teilstrichstärke", 3);
	TeilstrichLaenge=kc->readNumEntry("Teilstrichlänge", 10);
        tmp_color.setRgb( 0, 0, 0 );
	AchsenFarbe=kc->readColorEntry( "Achsenfarbe", &tmp_color ).rgb();
	if(kc->readNumEntry("Beschriftung", 1)) mode|=BESCHRIFTUNG;
	
	// Standardwerte für das Gitter

	kc->setGroup("Gitter");
	GitterDicke=kc->readNumEntry("Gitterstärke", 1);
	g_mode=kc->readNumEntry("Mode", 1);
	tmp_color.setRgb( 192, 192, 192 );
        GitterFarbe=kc->readColorEntry("Gitterfarbe", &tmp_color ).rgb();

	// Standardwerte für die Graphen

	kc->setGroup("Graphen");
	ps.dicke0=kc->readNumEntry("Linienstärke", 5);
        tmp_color.setRgb( 255, 0, 0 );
	ps.fktext[0].farbe0=kc->readColorEntry("Farbe0", &tmp_color ).rgb(); 
        tmp_color.setRgb( 0, 255, 0 );
        ps.fktext[1].farbe0=kc->readColorEntry("Farbe1", &tmp_color ).rgb();
        tmp_color.setRgb( 0, 0, 255 );
	ps.fktext[2].farbe0=kc->readColorEntry("Farbe2", &tmp_color ).rgb(); 
        tmp_color.setRgb( 255, 0, 255 );
        ps.fktext[3].farbe0=kc->readColorEntry("Farbe3", &tmp_color ).rgb();
        tmp_color.setRgb( 255, 255, 0 );
	ps.fktext[4].farbe0=kc->readColorEntry("Farbe4", &tmp_color ).rgb(); 
        tmp_color.setRgb( 0, 255, 255 );
        ps.fktext[5].farbe0=kc->readColorEntry("Farbe5", &tmp_color ).rgb();
        tmp_color.setRgb( 0, 128, 0 );
	ps.fktext[6].farbe0=kc->readColorEntry("Farbe6", &tmp_color ).rgb(); 
        tmp_color.setRgb( 0, 0, 128 );
        ps.fktext[7].farbe0=kc->readColorEntry("Farbe7", &tmp_color ).rgb(); 
        tmp_color.setRgb( 0, 0, 0 );
        ps.fktext[8].farbe0=kc->readColorEntry("Farbe8", &tmp_color ).rgb(); 
        ps.fktext[9].farbe0=kc->readColorEntry("Farbe9", &tmp_color ).rgb();

	for(ix=0; ix<ps.ufanz; ++ix) ps.delfkt(ix);
}
