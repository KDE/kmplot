// KDE includes
#include <kmessagebox.h>

// local includes
#include "misc.h"

KApplication *ka;
KConfig *kc;

XParser ps( 10, 200, 20 );

int mode,     // Diagrammodus
g_mode;  // Rastermodus

int koordx,     // 0 => [-8|+8]
koordy,     // 1 => [-5|+5]
// 2 => [0|+16]
// 3 => [0|+10]
AchsenDicke,
GitterDicke,
TeilstrichDicke,
TeilstrichLaenge;

double xmin,                      // min. x-Wert
xmax,                      // max. x-Wert
ymin,                   // min. y-Wert
ymax,                   // max. y-Wert
sw,       // Schrittweite
rsw,       // rel. Schrittweite
tlgx,       // x-Achsenteilung
tlgy,       // y-Achsenteilung
drskalx,       // x-Ausdruckskalierung
drskaly;     // y-Ausdruckskalierung

QString datei,      // Dateiname
xminstr,      // String für xmind
xmaxstr,      // String für xmaxd
yminstr,      // String für ymind
ymaxstr,      // String für ymaxd
tlgxstr,                  // String für tlgx
tlgystr,                  // String für tlgy
drskalxstr,               // String für drskalx
drskalystr;             // String für drskaly

QString font_header, font_axes; // Font family names

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

	if ( !coordToMinMax( koordx, xmin, xmax, xminstr, xmaxstr ) )
	{
		KMessageBox::error( 0, i18n( "Config file x-axis entry corrupt.\n"
		                             "Fall back to system defaults.\nCall Settings->Configure KmPlot..." ) );
		xminstr = "-2*pi";
		xmaxstr = "2*pi";
		koordx = 0;
		coordToMinMax( koordx, xmin, xmax, xminstr, xmaxstr );
	}
	if ( !coordToMinMax( koordy, ymin, ymax, yminstr, ymaxstr ) )
	{
		KMessageBox::error( 0, i18n( "Config file y-axis entry corrupt.\n"
		                             "Fall back to system defaults.\nCall Settings->Configure KmPlot..." ) );
		yminstr = "-2*pi";
		ymaxstr = "2*pi";
		koordy = 0;
		coordToMinMax( koordy, ymin, ymax, yminstr, ymaxstr );
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

	// font defaults
	if ( kc->hasGroup( "Fonts" ) )
	{
		kc->setGroup( "Fonts" );
		font_header = kc->readEntry( "Header Table", "Helvetica" );
		font_axes = kc->readEntry( "Axes", "Helvetica" );
	}
	else
	{
		font_header = "Helvetica";
		font_axes = "Helvetica";
	}

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

/*
 * evaluates the predefined axes settings (kkordx/y)
 */
bool coordToMinMax( const int koord, double &min, double &max, const QString minStr, const QString maxStr )
{
	bool ok = true;
	switch ( koord )
	{
	case 0:
		min = -8.0;
		max = 8.0;
		break;
	case 1:
		min = -5.0;
		max = 5.5;
		break;
	case 2:
		min = 0.0;
		max = 16.0;
		break;
	case 3:
		min = 0.0;
		max = 10.0;
		break;
	case 4:
		min = ps.eval( minStr );
		ok = ps.err == 0;
		max = ps.eval( maxStr );
		ok &= ps.err == 0;
	}
	ok &= min < max;
	return ok;
}

