// local includes
#include "misc.h"

KApplication *ka;
KConfig *kc;

XParser ps( 10, 200, 20 );

int mode,    // Diagrammodus
g_mode;  // Rastermodus

int koordx,    // 0 => [-8|+8]
koordy,    // 1 => [-5|+5]
// 2 => [0|+16]
// 3 => [0|+10]
AchsenDicke,
GitterDicke,
TeilstrichDicke,
TeilstrichLaenge;

double xmin,                     // min. x-Wert
xmax,                     // max. x-Wert
ymin,                  // min. y-Wert
ymax,                  // max. y-Wert
sw,      // Schrittweite
rsw,      // rel. Schrittweite
tlgx,      // x-Achsenteilung
tlgy,      // y-Achsenteilung
drskalx,      // x-Ausdruckskalierung
drskaly;     // y-Ausdruckskalierung

QString datei,     // Dateiname
xminstr,     // String f�r xmind
xmaxstr,     // String f�r xmaxd
yminstr,     // String f�r ymind
ymaxstr,     // String f�r ymaxd
tlgxstr,                 // String f�r tlgx
tlgystr,                 // String f�r tlgy
drskalxstr,              // String f�r drskalx
drskalystr;             // String f�r drskaly

QRgb AchsenFarbe,
GitterFarbe;

bool printtable;		// header table printing option


void init()
{
    int ix;

    QColor tmp_color;
    mode = ACHSEN | PFEILE | EXTRAHMEN;
    rsw = 1.;
    datei = "";

    // axes defaults

    kc->setGroup( "Axes" );
	koordx = kc->readNumEntry( "XCoord", 0 );
	koordy = kc->readNumEntry( "YCoord", 0 );
    xminstr = kc->readEntry( "Xmin", "-2*pi" );
    xmaxstr = kc->readEntry( "Xmax", "2*pi" );
    yminstr = kc->readEntry( "Ymin", "-2*pi" );
    ymaxstr = kc->readEntry( "Ymax", "2*pi" );

    switch ( koordx )
    {
    case 0:
        xmin = -8.0;
        xmax = 8.0;
		break;
    case 1:
        xmin = -5.0;
        xmax = 5.0;
		break;
    case 2:
        xmin = 0.0;
        xmax = 16.0;
		break;
    case 3:
        xmin = 0.0;
        xmax = 10.0;
		break;
    case 4:
        xmin = ps.eval( xminstr );
		xmax = ps.eval( xmaxstr );
	}
	
    switch ( koordy )
    {
    case 0:
        ymin = -8.0;
        ymax = 8.0;
		break;
    case 1:
        ymin = -5.0;
        ymax = 5.0;
		break;
    case 2:
        ymin = 0.0;
        ymax = 16.0;
		break;
    case 3:
        ymin = 0.0;
        ymax = 10.0;
		break;
    case 4:
        ymin = ps.eval( yminstr );
		ymax = ps.eval( ymaxstr );
    }

	tlgxstr = kc->readEntry( "tlgx", "1" );
    tlgx = ps.eval( tlgxstr );
    tlgystr = kc->readEntry( "tlgy", "1" );
    tlgy = ps.eval( tlgystr );

    drskalxstr = kc->readEntry( "drskalx", "1" );
    drskalx = ps.eval( drskalxstr );
    drskalystr = kc->readEntry( "drskaly", "1" );
    drskaly = ps.eval( drskalystr );

    AchsenDicke = kc->readNumEntry( "Axes Width", 5 );
    TeilstrichDicke = kc->readNumEntry( "Tic Width", 3 );
    TeilstrichLaenge = kc->readNumEntry( "Tic Length", 10 );
    tmp_color.setRgb( 0, 0, 0 );
    AchsenFarbe = kc->readColorEntry( "Color", &tmp_color ).rgb();
    if ( kc->readBoolEntry( "Labeled", true ) )
        mode |= BESCHRIFTUNG;

    // grid defaults

    kc->setGroup( "Grid" );
    GitterDicke = kc->readNumEntry( "Line Width", 1 );
    g_mode = kc->readNumEntry( "Mode", 1 );
    tmp_color.setRgb( 192, 192, 192 );
    GitterFarbe = kc->readColorEntry( "Color", &tmp_color ).rgb();

    // graph defaults

    kc->setGroup( "Graphs" );
    ps.dicke0 = kc->readNumEntry( "Line Width", 5 );
    tmp_color.setRgb( 255, 0, 0 );
    ps.fktext[ 0 ].farbe0 = kc->readColorEntry( "Color0", &tmp_color ).rgb();
    tmp_color.setRgb( 0, 255, 0 );
    ps.fktext[ 1 ].farbe0 = kc->readColorEntry( "Color1", &tmp_color ).rgb();
    tmp_color.setRgb( 0, 0, 255 );
    ps.fktext[ 2 ].farbe0 = kc->readColorEntry( "Color2", &tmp_color ).rgb();
    tmp_color.setRgb( 255, 0, 255 );
    ps.fktext[ 3 ].farbe0 = kc->readColorEntry( "Color3", &tmp_color ).rgb();
    tmp_color.setRgb( 255, 255, 0 );
    ps.fktext[ 4 ].farbe0 = kc->readColorEntry( "Color4", &tmp_color ).rgb();
    tmp_color.setRgb( 0, 255, 255 );
    ps.fktext[ 5 ].farbe0 = kc->readColorEntry( "Color5", &tmp_color ).rgb();
    tmp_color.setRgb( 0, 128, 0 );
    ps.fktext[ 6 ].farbe0 = kc->readColorEntry( "Color6", &tmp_color ).rgb();
    tmp_color.setRgb( 0, 0, 128 );
    ps.fktext[ 7 ].farbe0 = kc->readColorEntry( "Color7", &tmp_color ).rgb();
    tmp_color.setRgb( 0, 0, 0 );
    ps.fktext[ 8 ].farbe0 = kc->readColorEntry( "Color8", &tmp_color ).rgb();
    ps.fktext[ 9 ].farbe0 = kc->readColorEntry( "Color9", &tmp_color ).rgb();

    for ( ix = 0; ix < ps.ufanz; ++ix )
        ps.delfkt( ix );
	printtable = true;
}
